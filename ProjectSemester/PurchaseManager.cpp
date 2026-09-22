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
    string& errorMessage)
{
    if (lines.empty())
    {
        errorMessage =
            "An invoice needs at least one priced delivery.";

        return nullptr;
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

    auto invoice = make_unique<PurchaseInvoice>(
        id,
        date,
        currency,
        exchangeRate,
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

    invoices.push_back(
        std::move(invoice));

    save();

    if (movementLogger != nullptr)
    {
        movementLogger->logSystemEvent(
            "PURCHASE INVOICE CREATED",
            "ID: " + id +
            " | Lines: " + to_string(lines.size()) +
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

    invoice->setDate(date);
    invoice->setCurrency(currency);
    invoice->setExchangeRate(exchangeRate);
    invoice->setCustomsCost(customsCost);
    invoice->setFreightCost(freightCost);
    invoice->setComment(sanitizeField(comment));
    invoice->setLines(lines);

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
            entry.unitPriceEUR = invoice->getLineUnitPriceEUR(i);
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
//   lines
//
// lines: each as "procurementOrderID,materialID,receiptIndex,
//   receivedQuantity,unitCost", several joined by "~"

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

        file << invoice->getID() << "|"
            << invoice->getDate() << "|"
            << invoice->getCurrency() << "|"
            << invoice->getExchangeRate() << "|"
            << invoice->getCustomsCost() << "|"
            << invoice->getFreightCost() << "|"
            << invoice->getComment() << "|"
            << linesField
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
