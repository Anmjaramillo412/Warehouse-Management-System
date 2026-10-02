#include "PurchaseManager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;


// ================================================================
// FIELD SANITIZING (free text fields must not break the format)
// ================================================================

static string sanitizeField(const string& value)
{
    string result = value;

    replace(result.begin(), result.end(), '|', ' ');
    replace(result.begin(), result.end(), '~', ' ');
    replace(result.begin(), result.end(), ',', ';');
    replace(result.begin(), result.end(), '\n', ' ');
    replace(result.begin(), result.end(), '\r', ' ');

    return result;
}


// ================================================================
// SPLIT HELPER
// ================================================================

static vector<string> splitBy(
    const string& value,
    char delimiter)
{
    vector<string> result;

    stringstream stream(value);

    string token;

    while (getline(stream, token, delimiter))
    {
        result.push_back(token);
    }

    return result;
}


// ================================================================
// CONSTRUCTOR
// ================================================================

PurchaseManager::PurchaseManager(
    ProcurementManager* procManager,
    MovementLogger* logger,
    string file,
    string configFile)
{
    procurementManager = procManager;
    movementLogger = logger;
    materialManager = nullptr;

    filename = file;
    configFilename = configFile;

    nextNumber = 1;

    // 1.0 is a safe default (equivalent to "not converting anything"
    // until the real USD -> EUR rate is set in Purchase Settings).
    defaultExchangeRate = 1.0;

    load();
}


void PurchaseManager::setProcurementManager(
    ProcurementManager* manager)
{
    procurementManager = manager;
}


void PurchaseManager::setMovementLogger(
    MovementLogger* logger)
{
    movementLogger = logger;
}


void PurchaseManager::setMaterialManager(
    MaterialManager* manager)
{
    materialManager = manager;
}


// ================================================================
// GENERATE NEXT ID
// ================================================================

string PurchaseManager::generateNextID()
{
    stringstream stream;

    stream << "INV-"
        << setfill('0') << setw(6)
        << nextNumber;

    nextNumber++;

    return stream.str();
}


// ================================================================
// LINE SUPPLIER NAME (for Other Costs amortization)
// ================================================================

string PurchaseManager::getLineSupplierName(
    const PurchaseInvoiceLine& line) const
{
    if (materialManager == nullptr)
    {
        return "";
    }

    Material* material =
        materialManager->findMaterial(line.materialID);

    if (material == nullptr || material->getSupplier() == nullptr)
    {
        return "";
    }

    return material->getSupplier()->getName();
}


// ================================================================
// PENDING RECEIPTS
// ================================================================

vector<PendingPurchaseReceipt> PurchaseManager::getPendingReceipts() const
{
    vector<PendingPurchaseReceipt> pending;

    if (procurementManager == nullptr)
    {
        return pending;
    }

    for (const auto& order : procurementManager->getOrders())
    {
        const auto& receipts =
            order->getReceipts();

        for (size_t i = 0; i < receipts.size(); i++)
        {
            bool alreadyPriced = false;

            for (const auto& invoice : invoices)
            {
                for (const auto& line : invoice->getLines())
                {
                    if (line.procurementOrderID == order->getID() &&
                        line.materialID == order->getMaterialID() &&
                        line.receiptIndex == (int)i)
                    {
                        alreadyPriced = true;
                        break;
                    }
                }

                if (alreadyPriced)
                {
                    break;
                }
            }

            if (alreadyPriced)
            {
                continue;
            }

            PendingPurchaseReceipt pendingReceipt;

            pendingReceipt.procurementOrderID = order->getID();
            pendingReceipt.materialID = order->getMaterialID();
            pendingReceipt.receiptIndex = (int)i;
            pendingReceipt.receiptDate = receipts[i].receiptDate;
            pendingReceipt.receivedQuantity = receipts[i].receivedQuantity;
            pendingReceipt.comment = receipts[i].comment;

            pending.push_back(pendingReceipt);
        }
    }

    return pending;
}


// ================================================================
// CREATE INVOICE
// ================================================================

PurchaseInvoice* PurchaseManager::createInvoice(
    const string& date,
    const string& currency,
    double exchangeRate,
    double customsCost,
    double freightCost,
    const string& comment,
    const vector<PurchaseInvoiceLine>& lines,
    const vector<PurchaseInvoiceOtherCost>& otherCosts,
    string& errorMessage)
{
    if (lines.empty())
    {
        errorMessage =
            "An invoice needs at least one priced delivery.";

        return nullptr;
    }

    for (const auto& otherCost : otherCosts)
    {
        if (otherCost.supplierName.empty())
        {
            errorMessage =
                "Each Other Cost needs a Supplier.";

            return nullptr;
        }

        if (otherCost.amountEUR < 0.0)
        {
            errorMessage =
                "Other Cost amount cannot be negative (" +
                otherCost.supplierName + ").";

            return nullptr;
        }
    }

    // Validate every line against Procurement and against every
    // other already-priced receipt (this invoice's own lines
    // included, so the same receipt cannot be listed twice either).
    //
    // A line with receiptIndex < 0 is a manual price adjustment - a
    // direct price registration for a material that has no
    // Procurement delivery behind it (e.g. a price known from another
    // source, or setting up initial prices for several materials at
    // once, the way a real supplier invoice with several materials,
    // customs and freight would arrive). It carries procurementOrderID
    // "MANUAL" and a receivedQuantity given directly by the user, and
    // skips the pending-receipt match entirely - it is never "already
    // priced" by Procurement, so there is nothing there to check it
    // against.

    vector<PendingPurchaseReceipt> pendingReceipts =
        getPendingReceipts();

    for (const auto& line : lines)
    {
        if (line.unitCost < 0.0)
        {
            errorMessage =
                "Unit cost cannot be negative (" +
                line.materialID + ").";

            return nullptr;
        }

        bool isManual =
            (line.receiptIndex < 0);

        if (isManual)
        {
            if (line.procurementOrderID != "MANUAL")
            {
                errorMessage =
                    "Invalid manual price adjustment line for " +
                    line.materialID + ".";

                return nullptr;
            }

            if (line.receivedQuantity <= 0)
            {
                errorMessage =
                    "Quantity must be greater than zero (" +
                    line.materialID + ").";

                return nullptr;
            }

            continue;
        }

        if (procurementManager == nullptr)
        {
            errorMessage =
                "Procurement data is not available.";

            return nullptr;
        }

        bool foundPending = false;

        for (const auto& pendingReceipt : pendingReceipts)
        {
            if (pendingReceipt.procurementOrderID == line.procurementOrderID &&
                pendingReceipt.materialID == line.materialID &&
                pendingReceipt.receiptIndex == line.receiptIndex)
            {
                if (pendingReceipt.receivedQuantity != line.receivedQuantity)
                {
                    errorMessage =
                        "Received quantity does not match Procurement "
                        "for " + line.procurementOrderID + " / " +
                        line.materialID + ".";

                    return nullptr;
                }

                foundPending = true;
                break;
            }
        }

        if (!foundPending)
        {
            errorMessage =
                "Delivery not found or already priced: " +
                line.procurementOrderID + " / " + line.materialID + ".";

            return nullptr;
        }
    }

    // Refuse duplicate lines within this same invoice (the same
    // receipt selected twice in one submission).

    for (size_t i = 0; i < lines.size(); i++)
    {
        for (size_t j = i + 1; j < lines.size(); j++)
        {
            if (lines[i].procurementOrderID == lines[j].procurementOrderID &&
                lines[i].materialID == lines[j].materialID &&
                lines[i].receiptIndex == lines[j].receiptIndex)
            {
                errorMessage =
                    "The same delivery was selected twice: " +
                    lines[i].procurementOrderID + " / " +
                    lines[i].materialID + ".";

                return nullptr;
            }
        }
    }

    string id =
        generateNextID();

    // A EUR invoice is always exchangeRate 1.0 - forced here rather
    // than trusted from the caller, so a stale/mismatched rate left
    // over from switching the Currency field away from USD and back
    // (or any other client-side slip) can never silently deflate or
    // inflate every price derived from this invoice.
    double effectiveExchangeRate =
        (currency == "EUR") ? 1.0 : exchangeRate;

    auto invoice = make_unique<PurchaseInvoice>(
        id,
        date,
        currency,
        effectiveExchangeRate,
        customsCost,
        freightCost,
        sanitizeField(comment));

    for (const auto& line : lines)
    {
        PurchaseInvoiceLine sanitizedLine = line;

        sanitizedLine.materialID =
            sanitizeField(sanitizedLine.materialID);

        invoice->addLine(sanitizedLine);
    }

    for (const auto& otherCost : otherCosts)
    {
        PurchaseInvoiceOtherCost sanitizedOtherCost = otherCost;

        sanitizedOtherCost.supplierName =
            sanitizeField(sanitizedOtherCost.supplierName);
        sanitizedOtherCost.comment =
            sanitizeField(sanitizedOtherCost.comment);

        invoice->addOtherCost(sanitizedOtherCost);
    }

    invoices.push_back(
        std::move(invoice));

    save();

    if (movementLogger != nullptr)
    {
        movementLogger->logSystemEvent(
            "PURCHASE INVOICE CREATED",
            "ID: " + id +
            " | Lines: " + to_string(lines.size()) +
            " | Other Costs: " + to_string(otherCosts.size()) +
            " | Currency: " + currency);
    }

    return invoices.back().get();
}


// ================================================================
// FIND INVOICE
// ================================================================

PurchaseInvoice* PurchaseManager::findInvoice(
    const string& id)
{
    for (const auto& invoice : invoices)
    {
        if (invoice->getID() == id)
        {
            return invoice.get();
        }
    }

    return nullptr;
}


// ================================================================
// UPDATE INVOICE (rework its costs, e.g. once the customs bill
// arrives separately from the supplier invoice)
// ================================================================

bool PurchaseManager::updateInvoice(
    const string& id,
    const string& date,
    const string& currency,
    double exchangeRate,
    double customsCost,
    double freightCost,
    const string& comment,
    const vector<PurchaseInvoiceLine>& lines,
    const vector<PurchaseInvoiceOtherCost>& otherCosts,
    string& errorMessage)
{
    PurchaseInvoice* invoice =
        findInvoice(id);

    if (invoice == nullptr)
    {
        errorMessage =
            "Purchase Invoice not found.";

        return false;
    }

    if (date.empty())
    {
        errorMessage =
            "Invoice Date is required.";

        return false;
    }

    if (currency != "EUR" && currency != "USD")
    {
        errorMessage =
            "Currency must be EUR or USD.";

        return false;
    }

    if (exchangeRate <= 0.0)
    {
        errorMessage =
            "Exchange Rate must be greater than zero.";

        return false;
    }

    if (customsCost < 0.0 || freightCost < 0.0)
    {
        errorMessage =
            "Customs Cost and Freight Cost cannot be negative.";

        return false;
    }

    // The deliveries themselves are fixed once an invoice exists -
    // only their unitCost can be reworked here. Everything else
    // about a line (which delivery it is, its quantity) must match
    // exactly what is already on the invoice, in the same order.

    const vector<PurchaseInvoiceLine>& existingLines =
        invoice->getLines();

    if (lines.size() != existingLines.size())
    {
        errorMessage =
            "Cannot change the deliveries on an existing invoice - "
            "only its costs.";

        return false;
    }

    for (size_t i = 0; i < lines.size(); i++)
    {
        if (lines[i].procurementOrderID != existingLines[i].procurementOrderID ||
            lines[i].materialID != existingLines[i].materialID ||
            lines[i].receiptIndex != existingLines[i].receiptIndex ||
            lines[i].receivedQuantity != existingLines[i].receivedQuantity)
        {
            errorMessage =
                "Cannot change the deliveries on an existing invoice - "
                "only its costs.";

            return false;
        }

        if (lines[i].unitCost < 0.0)
        {
            errorMessage =
                "Unit cost cannot be negative (" +
                lines[i].materialID + ").";

            return false;
        }
    }

    for (const auto& otherCost : otherCosts)
    {
        if (otherCost.supplierName.empty())
        {
            errorMessage =
                "Each Other Cost needs a Supplier.";

            return false;
        }

        if (otherCost.amountEUR < 0.0)
        {
            errorMessage =
                "Other Cost amount cannot be negative (" +
                otherCost.supplierName + ").";

            return false;
        }
    }

    // Same EUR -> rate 1.0 guard as createInvoice() above - this is
    // also how an existing invoice with a bad stored rate (e.g. one
    // created before this guard existed) gets corrected: editing it
    // and saving, even with no other change, now always writes 1.0
    // for a EUR invoice regardless of what was there before.
    double effectiveExchangeRate =
        (currency == "EUR") ? 1.0 : exchangeRate;

    invoice->setDate(date);
    invoice->setCurrency(currency);
    invoice->setExchangeRate(effectiveExchangeRate);
    invoice->setCustomsCost(customsCost);
    invoice->setFreightCost(freightCost);
    invoice->setComment(sanitizeField(comment));
    invoice->setLines(lines);

    vector<PurchaseInvoiceOtherCost> sanitizedOtherCosts;

    for (const auto& otherCost : otherCosts)
    {
        PurchaseInvoiceOtherCost sanitizedOtherCost = otherCost;

        sanitizedOtherCost.supplierName =
            sanitizeField(sanitizedOtherCost.supplierName);
        sanitizedOtherCost.comment =
            sanitizeField(sanitizedOtherCost.comment);

        sanitizedOtherCosts.push_back(sanitizedOtherCost);
    }

    invoice->setOtherCosts(sanitizedOtherCosts);

    save();

    if (movementLogger != nullptr)
    {
        movementLogger->logSystemEvent(
            "PURCHASE INVOICE UPDATED",
            "ID: " + id);
    }

    return true;
}


// ================================================================
// DELETE INVOICE
// ================================================================

bool PurchaseManager::deleteInvoice(
    const string& id)
{
    for (auto it = invoices.begin(); it != invoices.end(); ++it)
    {
        if ((*it)->getID() == id)
        {
            invoices.erase(it);

            save();

            if (movementLogger != nullptr)
            {
                movementLogger->logSystemEvent(
                    "PURCHASE INVOICE DELETED",
                    "ID: " + id);
            }

            return true;
        }
    }

    return false;
}


const vector<unique_ptr<PurchaseInvoice>>&
PurchaseManager::getInvoices() const
{
    return invoices;
}


// ================================================================
// "OTHER COSTS" AMORTIZATION
// ================================================================
// Orders invoices oldest-to-newest by (date, ID) - ID as the
// tie-breaker since IDs are assigned in creation order, so two
// invoices dated the same day still get a stable, deterministic
// sequence.

static bool invoiceIsAtOrBefore(
    const PurchaseInvoice* a,
    const PurchaseInvoice* b)
{
    if (a->getDate() != b->getDate())
    {
        return a->getDate() < b->getDate();
    }

    return a->getID() <= b->getID();
}


double PurchaseManager::getLineOtherCostAdditionEUR(
    const PurchaseInvoice* invoice,
    size_t lineIndex) const
{
    if (invoice == nullptr ||
        lineIndex >= invoice->getLines().size())
    {
        return 0.0;
    }

    const PurchaseInvoiceLine& line =
        invoice->getLines()[lineIndex];

    string lineSupplierName =
        getLineSupplierName(line);

    if (lineSupplierName.empty())
    {
        return 0.0;
    }

    double totalAddition = 0.0;

    // Every Other Cost entry, on ANY invoice, that names this line's
    // Supplier can contribute - not just entries on THIS invoice.
    // An invoice-only entry only ever affects the one invoice it was
    // entered on; a Supplier-wide entry is entered once (e.g. on the
    // invoice that actually carried the one-time charge) and from
    // then on applies automatically to every later invoice for that
    // Supplier too, with an ever-growing cumulative quantity shrinking
    // its per-unit share each time - without ever being re-entered,
    // and without ever changing what an earlier invoice already
    // computed (see PurchaseInvoiceOtherCost's worked example: a 600
    // EUR charge on a first 400-unit invoice adds 1.50 EUR/unit there;
    // a second, unrelated 1000-unit invoice from the same Supplier,
    // with no Other Cost entry of its own, still gets 600 / 1400 =
    // 0.43 EUR/unit from that same original entry).

    for (const auto& sourceInvoice : invoices)
    {
        for (const auto& otherCost : sourceInvoice->getOtherCosts())
        {
            if (otherCost.supplierName != lineSupplierName)
            {
                continue;
            }

            if (otherCost.supplierWide)
            {
                // Only applies from the invoice it was entered on
                // forward in time - never to an invoice that predates
                // it (nothing to "amortize into" before the charge
                // existed).
                if (!invoiceIsAtOrBefore(
                        sourceInvoice.get(), invoice))
                {
                    continue;
                }
            }
            else
            {
                // Invoice-only: applies to nothing but the one
                // invoice it was entered on.
                if (sourceInvoice.get() != invoice)
                {
                    continue;
                }
            }

            double quantityBasis = 0.0;

            if (otherCost.supplierWide)
            {
                // Cumulative quantity of this Supplier's materials
                // across every invoice up to and including the one
                // being PRICED (not the one the cost was entered on) -
                // this is what makes the per-unit share keep shrinking
                // on later invoices with no Other Cost entry of their
                // own.
                for (const auto& qtyInvoice : invoices)
                {
                    if (!invoiceIsAtOrBefore(
                            qtyInvoice.get(), invoice))
                    {
                        continue;
                    }

                    const auto& qtyLines =
                        qtyInvoice->getLines();

                    for (size_t j = 0; j < qtyLines.size(); j++)
                    {
                        if (getLineSupplierName(qtyLines[j]) ==
                            lineSupplierName)
                        {
                            quantityBasis +=
                                qtyLines[j].receivedQuantity;
                        }
                    }
                }
            }
            else
            {
                // Invoice-only: just this Supplier's quantity on the
                // one invoice the cost was entered on (== invoice,
                // since we only reach here when sourceInvoice ==
                // invoice).
                const auto& theseLines =
                    sourceInvoice->getLines();

                for (size_t j = 0; j < theseLines.size(); j++)
                {
                    if (getLineSupplierName(theseLines[j]) ==
                        lineSupplierName)
                    {
                        quantityBasis +=
                            theseLines[j].receivedQuantity;
                    }
                }
            }

            if (quantityBasis > 0.0)
            {
                totalAddition +=
                    otherCost.amountEUR / quantityBasis;
            }
        }
    }

    return totalAddition;
}


double PurchaseManager::getLineLandedUnitPriceEUR(
    const PurchaseInvoice* invoice,
    size_t lineIndex) const
{
    if (invoice == nullptr)
    {
        return 0.0;
    }

    return invoice->getLineUnitPriceEUR(lineIndex) +
        getLineOtherCostAdditionEUR(invoice, lineIndex);
}


// ================================================================
// MATERIAL PRICE HISTORY
// ================================================================

vector<MaterialPriceHistoryEntry> PurchaseManager::getPriceHistory(
    const string& materialID) const
{
    vector<MaterialPriceHistoryEntry> history;

    for (const auto& invoice : invoices)
    {
        const auto& lines =
            invoice->getLines();

        for (size_t i = 0; i < lines.size(); i++)
        {
            if (lines[i].materialID != materialID)
            {
                continue;
            }

            MaterialPriceHistoryEntry entry;

            entry.date = invoice->getDate();
            entry.unitPriceEUR =
                getLineLandedUnitPriceEUR(invoice.get(), i);
            entry.originalCurrency = invoice->getCurrency();
            entry.originalUnitPrice = invoice->getLineUnitPrice(i);
            entry.exchangeRateUsed = invoice->getExchangeRate();
            entry.sourceInvoiceID = invoice->getID();
            entry.sourceProcurementOrderID = lines[i].procurementOrderID;

            history.push_back(entry);
        }
    }

    sort(
        history.begin(),
        history.end(),
        [](const MaterialPriceHistoryEntry& a,
           const MaterialPriceHistoryEntry& b)
        {
            return a.date < b.date;
        });

    return history;
}


bool PurchaseManager::hasPriceFor(
    const string& materialID) const
{
    return !getPriceHistory(materialID).empty();
}


double PurchaseManager::getCurrentUnitPriceEUR(
    const string& materialID) const
{
    vector<MaterialPriceHistoryEntry> history =
        getPriceHistory(materialID);

    if (history.empty())
    {
        return 0.0;
    }

    // getPriceHistory() sorts oldest-first, so the most recent
    // (current) price is the last entry.
    return history.back().unitPriceEUR;
}


// ================================================================
// PRODUCT COSTING
// ================================================================

ProductCostResult PurchaseManager::computeProductCost(
    const string& productID,
    const vector<BOMItem>& bom) const
{
    ProductCostResult result;

    result.productID = productID;
    result.totalCostEUR = 0.0;
    result.complete = true;

    for (const auto& item : bom)
    {
        ProductCostLine line;

        line.materialID = item.materialID;
        line.quantity = item.quantity;
        line.hasPrice = hasPriceFor(item.materialID);
        line.unitPriceEUR =
            line.hasPrice ?
            getCurrentUnitPriceEUR(item.materialID) : 0.0;
        line.lineCostEUR =
            line.unitPriceEUR * item.quantity;

        if (!line.hasPrice)
        {
            result.complete = false;
        }

        result.totalCostEUR += line.lineCostEUR;

        result.lines.push_back(line);
    }

    return result;
}


// ================================================================
// EXCHANGE RATE CONFIGURATION
// ================================================================

double PurchaseManager::getDefaultExchangeRate() const
{
    return defaultExchangeRate;
}


bool PurchaseManager::setDefaultExchangeRate(double rate)
{
    if (rate <= 0.0)
    {
        return false;
    }

    defaultExchangeRate = rate;

    ofstream file(configFilename);

    if (!file.is_open())
    {
        return false;
    }

    file << defaultExchangeRate << endl;

    file.close();

    if (movementLogger != nullptr)
    {
        movementLogger->logSystemEvent(
            "PURCHASE EXCHANGE RATE UPDATED",
            "New USD->EUR rate: " + to_string(defaultExchangeRate));
    }

    return true;
}


// ================================================================
// SAVE
// ================================================================
// Plain text format, one line per invoice:
//
// id|date|currency|exchangeRate|customsCost|freightCost|comment|
//   lines|otherCosts
//
// lines: each as "procurementOrderID,materialID,receiptIndex,
//   receivedQuantity,unitCost", several joined by "~"
//
// otherCosts: each as "supplierName,amountEUR,supplierWide,comment",
//   several joined by "~" - supplierWide is "1"/"0". Field is simply
//   absent (nothing after the last "|") on an invoice with none, same
//   as an empty lines field would be.

bool PurchaseManager::save()
{
    ofstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    for (const auto& invoice : invoices)
    {
        string linesField = "";

        const auto& lines =
            invoice->getLines();

        for (size_t i = 0; i < lines.size(); i++)
        {
            if (i > 0)
            {
                linesField += "~";
            }

            linesField +=
                lines[i].procurementOrderID + "," +
                lines[i].materialID + "," +
                to_string(lines[i].receiptIndex) + "," +
                to_string(lines[i].receivedQuantity) + "," +
                to_string(lines[i].unitCost);
        }

        string otherCostsField = "";

        const auto& otherCosts =
            invoice->getOtherCosts();

        for (size_t i = 0; i < otherCosts.size(); i++)
        {
            if (i > 0)
            {
                otherCostsField += "~";
            }

            otherCostsField +=
                otherCosts[i].supplierName + "," +
                to_string(otherCosts[i].amountEUR) + "," +
                (otherCosts[i].supplierWide ? "1" : "0") + "," +
                otherCosts[i].comment;
        }

        file << invoice->getID() << "|"
            << invoice->getDate() << "|"
            << invoice->getCurrency() << "|"
            << invoice->getExchangeRate() << "|"
            << invoice->getCustomsCost() << "|"
            << invoice->getFreightCost() << "|"
            << invoice->getComment() << "|"
            << linesField << "|"
            << otherCostsField
            << endl;
    }

    file.close();

    return true;
}


// ================================================================
// LOAD
// ================================================================

bool PurchaseManager::load()
{
    // Exchange rate config - a single number on its own tiny file.
    // Missing file just keeps the 1.0 default.

    ifstream configFile(configFilename);

    if (configFile.is_open())
    {
        double rate = 0.0;

        configFile >> rate;

        if (rate > 0.0)
        {
            defaultExchangeRate = rate;
        }

        configFile.close();
    }

    ifstream file(filename);

    if (!file.is_open())
    {
        // No file yet - not an error, just nothing to load.
        return true;
    }

    invoices.clear();

    int highestNumber = 0;

    string line;

    while (getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        vector<string> fields =
            splitBy(line, '|');

        if (fields.size() < 7)
        {
            continue;
        }

        auto invoice = make_unique<PurchaseInvoice>(
            fields[0],
            fields[1],
            fields[2],
            atof(fields[3].c_str()),
            atof(fields[4].c_str()),
            atof(fields[5].c_str()),
            fields[6]);

        if (fields.size() > 7 && !fields[7].empty())
        {
            vector<string> lineTokens =
                splitBy(fields[7], '~');

            for (const string& token : lineTokens)
            {
                vector<string> lineFields =
                    splitBy(token, ',');

                if (lineFields.size() < 5)
                {
                    continue;
                }

                PurchaseInvoiceLine invoiceLine;

                invoiceLine.procurementOrderID = lineFields[0];
                invoiceLine.materialID = lineFields[1];
                invoiceLine.receiptIndex = atoi(lineFields[2].c_str());
                invoiceLine.receivedQuantity = atoi(lineFields[3].c_str());
                invoiceLine.unitCost = atof(lineFields[4].c_str());

                invoice->addLine(invoiceLine);
            }
        }

        if (fields.size() > 8 && !fields[8].empty())
        {
            vector<string> otherCostTokens =
                splitBy(fields[8], '~');

            for (const string& token : otherCostTokens)
            {
                vector<string> otherCostFields =
                    splitBy(token, ',');

                if (otherCostFields.size() < 3)
                {
                    continue;
                }

                PurchaseInvoiceOtherCost otherCost;

                otherCost.supplierName = otherCostFields[0];
                otherCost.amountEUR = atof(otherCostFields[1].c_str());
                otherCost.supplierWide = (otherCostFields[2] == "1");
                otherCost.comment =
                    otherCostFields.size() > 3 ? otherCostFields[3] : "";

                invoice->addOtherCost(otherCost);
            }
        }

        // Track the highest existing "INV-######" number so new
        // invoices keep counting up instead of reusing IDs.

        string idNumberPart =
            fields[0].size() > 4 ?
            fields[0].substr(4) : "";

        int idNumber =
            atoi(idNumberPart.c_str());

        if (idNumber > highestNumber)
        {
            highestNumber = idNumber;
        }

        invoices.push_back(
            std::move(invoice));
    }

    file.close();

    nextNumber = highestNumber + 1;

    return true;
}


// ================================================================
// CLEAR
// ================================================================

void PurchaseManager::clear()
{
    invoices.clear();

    nextNumber = 1;
}
