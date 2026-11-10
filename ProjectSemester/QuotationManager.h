#pragma once

// ================================================================
// QUOTATION MANAGER (header-only)
// ----------------------------------------------------------------
// Supplier quotations ("QUO-######"), kept to document material
// prices in advance and to follow how they change over time, next to
// the Purchase Invoices (which hold the prices actually paid).
//
// A quotation belongs to ONE Supplier and has one line per Material.
// Every line carries PRICE TIERS BY QUANTITY ("from X units, unit
// price Y") - the quantity bought decides the price - plus the
// minimum order quantity and the lead time.
//
// A quotation never contains customs / freight: those only appear
// once an import happens (Purchase Invoice).
//
// CURRENT vs. PAST (automatic replacement): for a given Supplier and
// Material, the quotation with the latest issue date is the current
// one. Older ones are never deleted or overwritten - they stay on
// file as "Replaced" (with their PDF) and can still be consulted, and
// they are what price reconstruction for a past date relies on.
//
// Status (calculated, not stored):
//   Valid    - current, and not past its "valid until" date
//   Expired  - current, but past its "valid until" date. It is still
//              used for estimates, the screens just warn about it.
//   Replaced - every line was superseded by a newer quotation of the
//              same Supplier.
//
// Header-only on purpose: a new .cpp would also have to be added to
// the Visual Studio project.
// ================================================================

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <cmath>

#include "Quantity.h"
#include "MaterialManager.h"
#include "SupplierManager.h"
#include "MovementLogger.h"

using namespace std;


// ================================================================
// PRICE TIER
// ================================================================

struct QuotationTier
{
    // The tier applies from this quantity up to the next tier
    double minQuantity = 0.0;

    // In the currency of the quotation
    double unitPrice = 0.0;
};


// ================================================================
// QUOTATION LINE
// ================================================================

struct QuotationLine
{
    string materialID;

    // Minimum order quantity, 0 = none
    double moq = 0.0;

    // The quantity usually bought from this Supplier, as the start
    // quantity of one of the tiers below (0 = none marked). It
    // decides which tier's price is used as the reference price when
    // there is no newer invoice (see CostEstimator.h).
    double usualQuantity = 0.0;

    // Sorted by minQuantity (ascending), at least one
    vector<QuotationTier> tiers;
};


// ================================================================
// QUOTATION
// ================================================================

struct Quotation
{
    string id;

    string supplierName;

    // The number the Supplier gave to its own quotation
    string supplierQuoteNumber;

    // ISO dates (YYYY-MM-DD)
    string issueDate;

    // Empty = no end date stated
    string validUntil;

    // "EUR" or "USD"
    string currency = "EUR";

    // USD -> EUR rate (1.0 for EUR), same meaning as in invoices
    double exchangeRate = 1.0;

    string incoterm;

    // Lead time of the whole quotation, always in weeks (0 = not
    // stated) - every line of one quotation shares it
    int leadTimeWeeks = 0;

    string comment;

    // Quotation PDF (empty = none), saved under the Documents Folder
    string documentPath;

    vector<QuotationLine> lines;

    const QuotationLine* findLine(const string& materialID) const
    {
        for (const auto& line : lines)
        {
            if (line.materialID == materialID)
            {
                return &line;
            }
        }

        return nullptr;
    }
};


// ================================================================
// A (quotation, line) pair found by a lookup
// ================================================================

struct QuotationHit
{
    const Quotation* quotation = nullptr;

    const QuotationLine* line = nullptr;

    bool found() const
    {
        return quotation != nullptr && line != nullptr;
    }
};


// ================================================================
// MANAGER
// ================================================================

class QuotationManager
{
private:

    vector<unique_ptr<Quotation>> quotations;

    int nextNumber = 1;

    string filename;

    MaterialManager* materialManager;

    SupplierManager* supplierManager;

    MovementLogger* movementLogger;


    // ------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------

    static string encodeField(const string& value)
    {
        string result;

        const char* hex = "0123456789ABCDEF";

        for (unsigned char c : value)
        {
            if (c == '%' || c == '|' || c == '~' || c == ',' ||
                c == ';' || c == ':' || c == '\n' || c == '\r')
            {
                result += '%';
                result += hex[c >> 4];
                result += hex[c & 15];
            }
            else
            {
                result += static_cast<char>(c);
            }
        }

        return result;
    }

    static string decodeField(const string& value)
    {
        string result;

        for (size_t i = 0; i < value.size(); i++)
        {
            if (value[i] == '%' && i + 2 < value.size() &&
                isxdigit(static_cast<unsigned char>(value[i + 1])) &&
                isxdigit(static_cast<unsigned char>(value[i + 2])))
            {
                result += static_cast<char>(
                    strtol(value.substr(i + 1, 2).c_str(),
                        nullptr, 16));

                i += 2;
            }
            else
            {
                result += value[i];
            }
        }

        return result;
    }

    static vector<string> splitKeepEmpty(
        const string& value,
        char delimiter)
    {
        vector<string> result;

        string current;

        for (char c : value)
        {
            if (c == delimiter)
            {
                result.push_back(current);

                current.clear();
            }
            else
            {
                current += c;
            }
        }

        result.push_back(current);

        return result;
    }

    // Prices keep up to 6 decimals (unit prices of small parts)
    static string formatNumber(double value)
    {
        ostringstream stream;

        stream << setprecision(10) << value;

        return stream.str();
    }

    string nextID()
    {
        string number = to_string(nextNumber);

        nextNumber++;

        return "QUO-" +
            string(6 - min<size_t>(6, number.size()), '0') + number;
    }

    void log(
        const string& type,
        const string& details,
        const string& comment = "")
    {
        if (movementLogger != nullptr)
        {
            movementLogger->log(type, details, comment);
        }
    }

    // Is a newer quotation of the same Supplier covering this
    // Material? (later issue date; same date -> higher ID wins)
    bool supersededBy(
        const Quotation& quotation,
        const string& materialID,
        const Quotation& other) const
    {
        if (&other == &quotation ||
            other.supplierName != quotation.supplierName ||
            other.findLine(materialID) == nullptr)
        {
            return false;
        }

        if (other.issueDate != quotation.issueDate)
        {
            return other.issueDate > quotation.issueDate;
        }

        return other.id > quotation.id;
    }

    // Normalises and checks a quotation. Returns "" when valid,
    // otherwise the error text.
    string validate(Quotation& q) const
    {
        if (supplierManager == nullptr ||
            supplierManager->findSupplier(q.supplierName) == nullptr)
        {
            return "Supplier not found.";
        }

        if (!isValidDate(q.issueDate))
        {
            return "Issue date is required (YYYY-MM-DD).";
        }

        if (!q.validUntil.empty())
        {
            if (!isValidDate(q.validUntil))
            {
                return "Valid-until date must be YYYY-MM-DD.";
            }

            if (q.validUntil < q.issueDate)
            {
                return "Valid-until date cannot be before the issue date.";
            }
        }

        if (q.currency != "EUR" && q.currency != "USD")
        {
            return "Currency must be EUR or USD.";
        }

        if (q.currency == "EUR")
        {
            q.exchangeRate = 1.0;
        }
        else if (!(q.exchangeRate > 0.0) || !isfinite(q.exchangeRate))
        {
            return "Exchange rate must be greater than zero.";
        }

        if (q.leadTimeWeeks < 0)
        {
            return "Lead time (weeks) cannot be negative.";
        }

        if (q.lines.empty())
        {
            return "Add at least one material line.";
        }

        for (size_t i = 0; i < q.lines.size(); i++)
        {
            QuotationLine& line = q.lines[i];

            if (materialManager == nullptr ||
                materialManager->findMaterial(line.materialID) == nullptr)
            {
                return "Material " + line.materialID + " not found.";
            }

            for (size_t j = 0; j < i; j++)
            {
                if (q.lines[j].materialID == line.materialID)
                {
                    return "Material " + line.materialID +
                        " appears more than once.";
                }
            }

            if (line.moq < 0.0 || !isfinite(line.moq))
            {
                return "Material " + line.materialID +
                    ": minimum order quantity cannot be negative.";
            }

            if (line.tiers.empty())
            {
                return "Material " + line.materialID +
                    ": add at least one price tier.";
            }

            for (auto& tier : line.tiers)
            {
                tier.minQuantity = roundQty(tier.minQuantity);

                if (!(tier.minQuantity > 0.0))
                {
                    return "Material " + line.materialID +
                        ": every tier needs a quantity greater than zero.";
                }

                if (!(tier.unitPrice > 0.0) || !isfinite(tier.unitPrice))
                {
                    return "Material " + line.materialID +
                        ": every tier needs a unit price greater than zero.";
                }
            }

            line.usualQuantity = roundQty(line.usualQuantity);

            if (line.usualQuantity < 0.0)
            {
                return "Material " + line.materialID +
                    ": invalid usual purchase quantity.";
            }

            if (line.usualQuantity > 0.0)
            {
                bool matchesTier = false;

                for (const auto& tier : line.tiers)
                {
                    if (tier.minQuantity == line.usualQuantity)
                    {
                        matchesTier = true;
                    }
                }

                if (!matchesTier)
                {
                    return "Material " + line.materialID +
                        ": the usual purchase quantity must be one of "
                        "the tiers.";
                }
            }

            sort(line.tiers.begin(), line.tiers.end(),
                [](const QuotationTier& a, const QuotationTier& b)
                {
                    return a.minQuantity < b.minQuantity;
                });

            for (size_t t = 1; t < line.tiers.size(); t++)
            {
                if (line.tiers[t].minQuantity ==
                    line.tiers[t - 1].minQuantity)
                {
                    return "Material " + line.materialID +
                        ": two tiers start at the same quantity (" +
                        formatQty(line.tiers[t].minQuantity) + ").";
                }
            }
        }

        return "";
    }

    static string describe(const Quotation& q)
    {
        return q.id + " | " + q.supplierName + " | " + q.issueDate +
            " | " + to_string(q.lines.size()) + " line(s)";
    }

public:

    // ------------------------------------------------------------
    // Construction
    // ------------------------------------------------------------

    QuotationManager(
        MaterialManager* matManager = nullptr,
        SupplierManager* supManager = nullptr,
        MovementLogger* logger = nullptr,
        string file = "data/quotations.txt")
    {
        materialManager = matManager;

        supplierManager = supManager;

        movementLogger = logger;

        filename = file;

        load();
    }


    // ------------------------------------------------------------
    // Dates
    // ------------------------------------------------------------

    static bool isValidDate(const string& date)
    {
        if (date.size() != 10 || date[4] != '-' || date[7] != '-')
        {
            return false;
        }

        for (size_t i = 0; i < date.size(); i++)
        {
            if (i == 4 || i == 7)
            {
                continue;
            }

            if (!isdigit(static_cast<unsigned char>(date[i])))
            {
                return false;
            }
        }

        int month = atoi(date.substr(5, 2).c_str());

        int day = atoi(date.substr(8, 2).c_str());

        return month >= 1 && month <= 12 && day >= 1 && day <= 31;
    }

    static string today()
    {
        time_t now = time(nullptr);

        tm local{};

#ifdef _WIN32
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif

        char buffer[16];

        strftime(buffer, sizeof(buffer), "%Y-%m-%d", &local);

        return buffer;
    }


    // ------------------------------------------------------------
    // Access
    // ------------------------------------------------------------

    const vector<unique_ptr<Quotation>>& getQuotations() const
    {
        return quotations;
    }

    Quotation* findQuotation(const string& id)
    {
        for (auto& q : quotations)
        {
            if (q->id == id)
            {
                return q.get();
            }
        }

        return nullptr;
    }

    const Quotation* findQuotation(const string& id) const
    {
        for (const auto& q : quotations)
        {
            if (q->id == id)
            {
                return q.get();
            }
        }

        return nullptr;
    }


    // ------------------------------------------------------------
    // Current / past, status
    // ------------------------------------------------------------

    // Is this line of the quotation the current one for its
    // Supplier and Material (no newer quotation covers it)?
    bool isLineCurrent(
        const Quotation& quotation,
        const string& materialID) const
    {
        for (const auto& other : quotations)
        {
            if (supersededBy(quotation, materialID, *other))
            {
                return false;
            }
        }

        return true;
    }

    // "Valid", "Expired" or "Replaced" (see the header comment)
    string status(const Quotation& quotation) const
    {
        bool anyCurrent = false;

        for (const auto& line : quotation.lines)
        {
            if (isLineCurrent(quotation, line.materialID))
            {
                anyCurrent = true;

                break;
            }
        }

        if (!anyCurrent)
        {
            return "Replaced";
        }

        if (!quotation.validUntil.empty() &&
            quotation.validUntil < today())
        {
            return "Expired";
        }

        return "Valid";
    }

    // The quotation line of this Supplier and Material that was in
    // force on asOfDate (the one with the latest issue date up to and
    // including that day). Empty asOfDate = today, i.e. the current
    // one. Expired ones are returned too - the caller decides how to
    // warn about them.
    QuotationHit findInForce(
        const string& materialID,
        const string& supplierName,
        const string& asOfDate = "") const
    {
        QuotationHit best;

        for (const auto& q : quotations)
        {
            if (q->supplierName != supplierName)
            {
                continue;
            }

            if (!asOfDate.empty() && q->issueDate > asOfDate)
            {
                continue;
            }

            const QuotationLine* line = q->findLine(materialID);

            if (line == nullptr)
            {
                continue;
            }

            if (!best.found() ||
                q->issueDate > best.quotation->issueDate ||
                (q->issueDate == best.quotation->issueDate &&
                    q->id > best.quotation->id))
            {
                best.quotation = q.get();

                best.line = line;
            }
        }

        return best;
    }

    // Every quotation line that mentions the Material, newest first
    // (current and past ones)
    vector<QuotationHit> findAllForMaterial(
        const string& materialID) const
    {
        vector<QuotationHit> hits;

        for (const auto& q : quotations)
        {
            const QuotationLine* line = q->findLine(materialID);

            if (line != nullptr)
            {
                QuotationHit hit;

                hit.quotation = q.get();

                hit.line = line;

                hits.push_back(hit);
            }
        }

        sort(hits.begin(), hits.end(),
            [](const QuotationHit& a, const QuotationHit& b)
            {
                if (a.quotation->issueDate != b.quotation->issueDate)
                {
                    return a.quotation->issueDate >
                        b.quotation->issueDate;
                }

                return a.quotation->id > b.quotation->id;
            });

        return hits;
    }


    // ------------------------------------------------------------
    // Price of a quantity
    // ------------------------------------------------------------

    // The tier that applies to this quantity: the one with the
    // highest start quantity that is not above it. A quantity below
    // the first tier gets the first tier (belowFirstTier = true, so
    // the screens can warn that it is under the quoted minimum).
    static const QuotationTier& tierFor(
        const QuotationLine& line,
        double quantity,
        bool* belowFirstTier = nullptr)
    {
        const QuotationTier* chosen = &line.tiers.front();

        bool below = quantity < chosen->minQuantity;

        for (const auto& tier : line.tiers)
        {
            if (tier.minQuantity <= quantity)
            {
                chosen = &tier;
            }
        }

        if (belowFirstTier != nullptr)
        {
            *belowFirstTier = below;
        }

        return *chosen;
    }

    // The quantity to price a material at when no purchase quantity
    // is given: the tier marked as the usual purchase quantity, or
    // the first tier when none is marked
    static double referenceQuantity(const QuotationLine& line)
    {
        return line.usualQuantity > 0.0 ?
            line.usualQuantity : line.tiers.front().minQuantity;
    }

    // Unit price of the tier in EUR
    static double unitPriceEUR(
        const Quotation& quotation,
        const QuotationTier& tier)
    {
        return tier.unitPrice * quotation.exchangeRate;
    }


    // ------------------------------------------------------------
    // Save / load (data/quotations.txt)
    // ------------------------------------------------------------
    // One line per quotation:
    //   id|supplier|supplierQuoteNo|issueDate|validUntil|currency|
    //   exchangeRate|incoterm|comment|documentPath|leadTimeWeeks|lines
    // lines joined by "~", each:
    //   materialID,moq,usualQuantity,tiers
    // (older files: lines without usualQuantity "materialID,moq,tiers",
    // and files without leadTimeWeeks have 11 fields with lines
    // "materialID,leadTimeDays,moq,tiers" - still read, see load())
    // tiers joined by ";", each: minQuantity:unitPrice
    // (free text is %-encoded so it can contain any character)

    bool save()
    {
        ofstream file(filename);

        if (!file.is_open())
        {
            return false;
        }

        for (const auto& q : quotations)
        {
            string linesField;

            for (size_t i = 0; i < q->lines.size(); i++)
            {
                const QuotationLine& line = q->lines[i];

                string tiersField;

                for (size_t t = 0; t < line.tiers.size(); t++)
                {
                    if (t > 0)
                    {
                        tiersField += ";";
                    }

                    tiersField +=
                        formatNumber(line.tiers[t].minQuantity) + ":" +
                        formatNumber(line.tiers[t].unitPrice);
                }

                if (i > 0)
                {
                    linesField += "~";
                }

                linesField +=
                    encodeField(line.materialID) + "," +
                    formatNumber(line.moq) + "," +
                    formatNumber(line.usualQuantity) + "," +
                    tiersField;
            }

            file << q->id << "|"
                << encodeField(q->supplierName) << "|"
                << encodeField(q->supplierQuoteNumber) << "|"
                << q->issueDate << "|"
                << q->validUntil << "|"
                << q->currency << "|"
                << formatNumber(q->exchangeRate) << "|"
                << encodeField(q->incoterm) << "|"
                << encodeField(q->comment) << "|"
                << encodeField(q->documentPath) << "|"
                << q->leadTimeWeeks << "|"
                << linesField
                << endl;
        }

        return true;
    }

    bool load()
    {
        ifstream file(filename);

        if (!file.is_open())
        {
            return true;
        }

        quotations.clear();

        int highest = 0;

        string text;

        while (getline(file, text))
        {
            if (text.empty())
            {
                continue;
            }

            if (text.back() == '\r')
            {
                text.pop_back();
            }

            vector<string> fields = splitKeepEmpty(text, '|');

            if (fields.size() < 11)
            {
                continue;
            }

            auto q = make_unique<Quotation>();

            q->id = fields[0];

            q->supplierName = decodeField(fields[1]);

            q->supplierQuoteNumber = decodeField(fields[2]);

            q->issueDate = fields[3];

            q->validUntil = fields[4];

            q->currency = fields[5];

            q->exchangeRate = atof(fields[6].c_str());

            q->incoterm = decodeField(fields[7]);

            q->comment = decodeField(fields[8]);

            q->documentPath = decodeField(fields[9]);

            // Older format (11 fields): lead time was stored per
            // line in days - keep the longest, rounded up to weeks
            bool oldFormat = fields.size() == 11;

            size_t linesIndex = oldFormat ? 10 : 11;

            int oldLeadDays = 0;

            if (!oldFormat)
            {
                q->leadTimeWeeks = atoi(fields[10].c_str());
            }

            if (!fields[linesIndex].empty())
            {
                for (const string& lineText :
                    splitKeepEmpty(fields[linesIndex], '~'))
                {
                    vector<string> parts =
                        splitKeepEmpty(lineText, ',');

                    if (parts.size() < (oldFormat ? 4u : 3u))
                    {
                        continue;
                    }

                    QuotationLine line;

                    line.materialID = decodeField(parts[0]);

                    if (oldFormat)
                    {
                        oldLeadDays = max(oldLeadDays,
                            atoi(parts[1].c_str()));
                    }

                    line.moq = atof(parts[oldFormat ? 2 : 1].c_str());

                    // Current format has the usual quantity before the
                    // tiers; the previous one went straight to them
                    size_t tiersIndex = oldFormat ? 3 : 2;

                    if (!oldFormat && parts.size() >= 4)
                    {
                        line.usualQuantity = atof(parts[2].c_str());

                        tiersIndex = 3;
                    }

                    for (const string& tierText :
                        splitKeepEmpty(parts[tiersIndex], ';'))
                    {
                        size_t colon = tierText.find(':');

                        if (colon == string::npos)
                        {
                            continue;
                        }

                        QuotationTier tier;

                        tier.minQuantity =
                            atof(tierText.substr(0, colon).c_str());

                        tier.unitPrice =
                            atof(tierText.substr(colon + 1).c_str());

                        line.tiers.push_back(tier);
                    }

                    if (!line.tiers.empty())
                    {
                        q->lines.push_back(line);
                    }
                }
            }

            if (oldFormat)
            {
                q->leadTimeWeeks = (oldLeadDays + 6) / 7;
            }

            if (q->id.size() > 4 && q->id.compare(0, 4, "QUO-") == 0)
            {
                highest = max(highest, atoi(q->id.c_str() + 4));
            }

            quotations.push_back(move(q));
        }

        nextNumber = highest + 1;

        return true;
    }


    // ------------------------------------------------------------
    // Create / update / delete
    // ------------------------------------------------------------

    // Validates and stores a new quotation (its ID is assigned here).
    // The quotation it replaces needs no change: the newest issue
    // date per Supplier and Material is current automatically.
    bool create(
        Quotation data,
        string& error,
        string* newID = nullptr)
    {
        error = validate(data);

        if (!error.empty())
        {
            return false;
        }

        auto q = make_unique<Quotation>(data);

        q->id = nextID();

        q->documentPath.clear();

        if (newID != nullptr)
        {
            *newID = q->id;
        }

        log("QUOTATION CREATED", describe(*q), q->comment);

        quotations.push_back(move(q));

        save();

        return true;
    }

    // Replaces everything except the ID and the PDF
    bool update(
        const string& id,
        Quotation data,
        string& error)
    {
        Quotation* existing = findQuotation(id);

        if (existing == nullptr)
        {
            error = "Quotation not found.";

            return false;
        }

        error = validate(data);

        if (!error.empty())
        {
            return false;
        }

        data.id = existing->id;

        data.documentPath = existing->documentPath;

        *existing = data;

        log("QUOTATION UPDATED", describe(*existing), existing->comment);

        save();

        return true;
    }

    bool remove(
        const string& id,
        string& error)
    {
        for (size_t i = 0; i < quotations.size(); i++)
        {
            if (quotations[i]->id == id)
            {
                log("QUOTATION DELETED", describe(*quotations[i]));

                quotations.erase(quotations.begin() + i);

                save();

                return true;
            }
        }

        error = "Quotation not found.";

        return false;
    }

    bool setDocumentPath(
        const string& id,
        const string& path)
    {
        Quotation* q = findQuotation(id);

        if (q == nullptr)
        {
            return false;
        }

        q->documentPath = path;

        log("QUOTATION DOCUMENT", q->id + " | PDF " +
            (path.empty() ? string("removed") : string("saved")));

        save();

        return true;
    }
};
