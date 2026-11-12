#include "WebServer.h"

#include <crow.h>
#include "AuthManager.h"
#include "Quantity.h"
#include "CostEstimator.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <ctime>
#include <algorithm>
#include <thread>
#include <chrono>

using namespace std;
namespace fs = std::filesystem;

// ================================================================
// QUANTITY HELPERS (decimal quantities, max. 3 decimals)
// ================================================================
// Reads a quantity from a JSON value. Returns -1 when the value is
// missing or not a number, so the existing "must be greater than
// zero" validations reject it.

static double jsonQuantity(const crow::json::rvalue& value)
{
    if (value.t() != crow::json::type::Number)
    {
        return -1.0;
    }

    double v = value.d();

    if (!std::isfinite(v))
    {
        return -1.0;
    }

    return v;
}

// ============================================================
// COST ESTIMATE HELPERS (see CostEstimator.h)
// ============================================================

// Optional ?quantity=N (how many Products will be produced) and
// ?date=YYYY-MM-DD (price reconstruction on a past date)
static void costQueryParams(
    const crow::request& req,
    double& quantity,
    string& asOfDate)
{
    // (?basis=quantity switches the price basis, see usualBasis())
    quantity = 1.0;

    asOfDate = "";

    const char* q = req.url_params.get("quantity");

    if (q != nullptr && *q != '\0')
    {
        double value = atof(q);

        if (std::isfinite(value) && value > 0.0)
        {
            quantity = value;
        }
    }

    const char* d = req.url_params.get("date");

    if (d != nullptr &&
        QuotationManager::isValidDate(string(d)))
    {
        asOfDate = d;
    }
}

// Product cost views price quotation-governed materials at the
// usual purchase quantity of the quotation by default;
// ?basis=quantity prices them at the quantity needed instead
static bool usualBasis(const crow::request& req)
{
    const char* b = req.url_params.get("basis");

    return !(b != nullptr && string(b) == "quantity");
}

static crow::json::wvalue estimateToJson(const MaterialCostEstimate& e)
{
    crow::json::wvalue j;

    j["source"] = e.source;
    j["estimated"] = e.estimated;
    j["unitPriceEUR"] = e.unitPriceEUR;
    j["sourceRef"] = e.sourceRef;
    j["sourceDate"] = e.sourceDate;
    j["supplierName"] = e.supplierName;
    j["basePriceEUR"] = e.basePriceEUR;
    j["importIncrementEUR"] = e.importIncrementEUR;
    j["hasImportIncrement"] = e.hasImportIncrement;
    j["incrementRef"] = e.incrementRef;
    j["belowFirstTier"] = e.belowFirstTier;
    j["belowMoq"] = e.belowMoq;
    j["quotationExpired"] = e.quotationExpired;
    j["tierMinQuantity"] = e.tierMinQuantity;
    j["usedUsualTier"] = e.usedUsualTier;
    j["usualMissing"] = e.usualMissing;
    j["hasQuoteInfo"] = e.hasQuoteInfo;
    j["quoteRef"] = e.quoteRef;
    j["quoteSupplier"] = e.quoteSupplier;
    j["quoteDate"] = e.quoteDate;
    j["quotePriceEUR"] = e.quotePriceEUR;
    j["quoteExpired"] = e.quoteExpired;
    j["quoteBelowFirstTier"] = e.quoteBelowFirstTier;
    j["quoteTierMinQuantity"] = e.quoteTierMinQuantity;
    j["hasLastInvoice"] = e.hasLastInvoice;
    j["lastInvoiceRef"] = e.lastInvoiceRef;
    j["lastInvoiceDate"] = e.lastInvoiceDate;
    j["lastInvoicePriceEUR"] = e.lastInvoicePriceEUR;

    return j;
}

// IDs of the invoices whose lines were priced from this quotation
static vector<string> quotationInvoiceIDs(
    WarehouseSystem* warehouseSystem,
    const string& quotationID)
{
    vector<string> ids;

    for (const auto& invoice :
        warehouseSystem->getPurchaseManager().getInvoices())
    {
        for (const auto& line : invoice->getLines())
        {
            if (line.quotationID == quotationID)
            {
                ids.push_back(invoice->getID());

                break;
            }
        }
    }

    return ids;
}

// Checks the quotation chosen for an invoice line and freezes the
// price its tier gives for the line's quantity (EUR). Returns ""
// when fine (or when no quotation was chosen), else the error text.
static string resolveLineQuotation(
    WarehouseSystem* warehouseSystem,
    PurchaseInvoiceLine& line)
{
    if (line.quotationID.empty())
    {
        line.quotedUnitPriceEUR = 0.0;

        return "";
    }

    const Quotation* quotation =
        warehouseSystem->getQuotationManager()
            .findQuotation(line.quotationID);

    if (quotation == nullptr)
    {
        return "Quotation " + line.quotationID + " not found.";
    }

    const QuotationLine* quotationLine =
        quotation->findLine(line.materialID);

    if (quotationLine == nullptr)
    {
        return "Quotation " + line.quotationID +
            " has no price for material " + line.materialID + ".";
    }

    const QuotationTier& tier =
        QuotationManager::tierFor(
            *quotationLine,
            line.receivedQuantity > 0.0 ?
            line.receivedQuantity :
            QuotationManager::referenceQuantity(*quotationLine));

    line.quotedUnitPriceEUR =
        QuotationManager::unitPriceEUR(*quotation, tier);

    return "";
}

static string invoiceLineQuotationID(const crow::json::rvalue& lineJson)
{
    if (!lineJson.has("quotationID") ||
        lineJson["quotationID"].t() != crow::json::type::String)
    {
        return "";
    }

    return string(lineJson["quotationID"].s());
}

// ============================================================
// QUOTATION HELPERS (see QuotationManager.h)
// ============================================================

static crow::json::wvalue quotationToJson(
    WarehouseSystem* warehouseSystem,
    const Quotation& q)
{
    QuotationManager& quotationManager =
        warehouseSystem->getQuotationManager();

    crow::json::wvalue item;

    item["id"] = q.id;

    item["supplier"] = q.supplierName;

    item["supplierQuoteNumber"] = q.supplierQuoteNumber;

    item["issueDate"] = q.issueDate;

    item["validUntil"] = q.validUntil;

    item["currency"] = q.currency;

    item["exchangeRate"] = q.exchangeRate;

    item["incoterm"] = q.incoterm;

    item["leadTimeWeeks"] = q.leadTimeWeeks;

    item["comment"] = q.comment;

    item["status"] = quotationManager.status(q);

    item["hasDocument"] =
        !q.documentPath.empty() && fs::exists(q.documentPath);

    // Invoices whose lines were priced from this quotation
    crow::json::wvalue::list invoiceList;

    for (const string& invoiceID :
        quotationInvoiceIDs(warehouseSystem, q.id))
    {
        invoiceList.push_back(invoiceID);
    }

    item["invoices"] = std::move(invoiceList);

    crow::json::wvalue::list lineList;

    for (const auto& line : q.lines)
    {
        crow::json::wvalue l;

        l["materialID"] = line.materialID;

        Material* material =
            warehouseSystem->getMaterialManager()
                .findMaterial(line.materialID);

        l["materialName"] =
            material != nullptr ? material->getName() : "";

        l["uom"] =
            material != nullptr ? material->getUoM() : "";

        l["moq"] = line.moq;

        l["usualQuantity"] = line.usualQuantity;

        l["current"] =
            quotationManager.isLineCurrent(q, line.materialID);

        crow::json::wvalue::list tierList;

        for (const auto& tier : line.tiers)
        {
            crow::json::wvalue t;

            t["minQuantity"] = tier.minQuantity;

            t["unitPrice"] = tier.unitPrice;

            t["unitPriceEUR"] =
                QuotationManager::unitPriceEUR(q, tier);

            tierList.push_back(std::move(t));
        }

        l["tiers"] = std::move(tierList);

        lineList.push_back(std::move(l));
    }

    item["lines"] = std::move(lineList);

    return item;
}

static string quotationJsonString(
    const crow::json::rvalue& body,
    const char* key)
{
    if (!body.has(key) ||
        body[key].t() != crow::json::type::String)
    {
        return "";
    }

    return string(body[key].s());
}

// Reads the quotation fields of a create / update request.
// Returns "" when readable, otherwise the error text.
static string quotationFromJson(
    const crow::json::rvalue& body,
    Quotation& q)
{
    q.supplierName = quotationJsonString(body, "supplier");

    q.supplierQuoteNumber =
        quotationJsonString(body, "supplierQuoteNumber");

    q.issueDate = quotationJsonString(body, "issueDate");

    q.validUntil = quotationJsonString(body, "validUntil");

    q.currency = quotationJsonString(body, "currency");

    if (q.currency.empty())
    {
        q.currency = "EUR";
    }

    q.incoterm = quotationJsonString(body, "incoterm");

    q.comment = quotationJsonString(body, "comment");

    q.leadTimeWeeks =
        body.has("leadTimeWeeks") &&
        body["leadTimeWeeks"].t() == crow::json::type::Number ?
        (int)body["leadTimeWeeks"].d() : 0;

    q.exchangeRate =
        body.has("exchangeRate") ?
        jsonQuantity(body["exchangeRate"]) : 1.0;

    if (!body.has("lines") ||
        body["lines"].t() != crow::json::type::List)
    {
        return "Add at least one material line.";
    }

    for (size_t i = 0; i < body["lines"].size(); i++)
    {
        const auto& l = body["lines"][i];

        QuotationLine line;

        line.materialID = quotationJsonString(l, "materialID");

        line.moq = l.has("moq") ? jsonQuantity(l["moq"]) : 0.0;

        line.usualQuantity =
            l.has("usualQuantity") &&
            l["usualQuantity"].t() == crow::json::type::Number ?
            jsonQuantity(l["usualQuantity"]) : 0.0;

        if (l.has("tiers") &&
            l["tiers"].t() == crow::json::type::List)
        {
            for (size_t t = 0; t < l["tiers"].size(); t++)
            {
                const auto& tj = l["tiers"][t];

                QuotationTier tier;

                tier.minQuantity =
                    tj.has("minQuantity") ?
                    jsonQuantity(tj["minQuantity"]) : -1.0;

                tier.unitPrice =
                    tj.has("unitPrice") ?
                    jsonQuantity(tj["unitPrice"]) : -1.0;

                line.tiers.push_back(tier);
            }
        }

        q.lines.push_back(line);
    }

    return "";
}

// Stock movements are suspended in a Warehouse while it has an open
// inventory count (see InventoryCountManager.h). Empty string when
// the Warehouse is free, otherwise the error text for the response.
static string warehouseLockedError(
    WarehouseSystem* warehouseSystem,
    int warehouseID)
{
    string countID =
        warehouseSystem->getInventoryCountManager()
        .getOpenCountID(warehouseID);

    if (countID.empty())
    {
        return "";
    }

    return "Warehouse " + to_string(warehouseID) +
        " is under inventory count " + countID +
        ". Stock movements are suspended until the count is "
        "applied or cancelled.";
}

// Empty string when OK, otherwise the error text for the response.
static string quantityDecimalsError(
    double value,
    const string& label)
{
    if (value > 0 && !hasValidQtyDecimals(value))
    {
        return label + " can have at most 3 decimals.";
    }

    return "";
}

// ================================================================
// READ FILE
// ================================================================

string readFile(const string& filename)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        return "";
    }

    stringstream buffer;

    buffer << file.rdbuf();

    return buffer.str();
}


// Same as readFile() but binary-safe (needed for the pdf.js files
// served from web/lib: text mode would alter bytes on Windows)
string readBinaryFile(const string& filename)
{
    ifstream file(filename, ios::binary);

    if (!file.is_open())
    {
        return "";
    }

    stringstream buffer;

    buffer << file.rdbuf();

    return buffer.str();
}


// ================================================================
// DATE HELPERS (plain "YYYY-MM-DD" strings, as used by <input
// type="date">) - used to work out whether it is still on time to
// place an order given a Projection's deadline and a Supplier's
// lead time.
// ================================================================

static bool parseISODate(
    const string& dateStr,
    tm& result)
{
    if (dateStr.size() < 10)
    {
        return false;
    }

    result = tm();

    result.tm_year =
        atoi(dateStr.substr(0, 4).c_str()) - 1900;

    result.tm_mon =
        atoi(dateStr.substr(5, 2).c_str()) - 1;

    result.tm_mday =
        atoi(dateStr.substr(8, 2).c_str());

    // Noon, to stay clear of daylight-saving edge cases when the
    // time_t gets shifted by whole days below.
    result.tm_hour = 12;

    return true;
}

static string formatISODate(
    time_t t)
{
    tm* localTm = localtime(&t);

    char buffer[11];

    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d",
        localTm);

    return string(buffer);
}

// Compares "today" against (deadline - leadTimeWeeks). Returns
// "On Time", "Overdue", "No Supplier" (material has none assigned)
// or "No Deadline" (Projection/date fields are empty or unreadable).
static void computeOrderTiming(
    const string& deadline,
    Supplier* supplier,
    string& status,
    string& orderByDate)
{
    if (supplier == nullptr)
    {
        status = "No Supplier";
        orderByDate = "";
        return;
    }

    tm deadlineTm;

    if (deadline.empty() ||
        !parseISODate(deadline, deadlineTm))
    {
        status = "No Deadline";
        orderByDate = "";
        return;
    }

    time_t deadlineTime =
        mktime(&deadlineTm);

    time_t orderByTime =
        deadlineTime -
        (time_t)(supplier->getLeadTimeWeeks() * 7 * 86400);

    orderByDate =
        formatISODate(orderByTime);


    time_t now = time(nullptr);

    tm nowTm = *localtime(&now);

    nowTm.tm_hour = 12;
    nowTm.tm_min = 0;
    nowTm.tm_sec = 0;

    time_t todayNoon =
        mktime(&nowTm);

    status =
        (todayNoon > orderByTime) ?
        "Overdue" : "On Time";
}


// ================================================================
// OUTSTANDING ORDERED QUANTITY (informational only)
// ================================================================
// Live sum of every still-outstanding Procurement Order quantity for
// a material inside one Warehouse, tied to any Projection or to none.
// This is shown next to "Qty to Order" purely so the user can judge
// how much more actually needs ordering - by design the two numbers
// are never netted against each other (see the Projection/Warehouse
// redesign notes in ProjectionManager.h/.cpp): the units already on
// order might be meant for a different Projection than the one being
// viewed, so the app shows both numbers and leaves that judgment call
// to the user instead of guessing at an attribution.

static double getOutstandingOrderedQuantity(
    const string& materialID,
    int warehouseID,
    ProcurementManager& procurementManager)
{
    double total = 0.0;

    for (const auto& order : procurementManager.getOrders())
    {
        if (order->getMaterialID() != materialID)
        {
            continue;
        }

        if (order->getWarehouseID() != warehouseID)
        {
            continue;
        }

        if (order->isClosed() ||
            order->isCancelled())
        {
            continue;
        }

        total = roundQty(total + order->getPendingQuantity());
    }

    return total;
}


// ================================================================
// AUTO SAVE (Data Management's "Auto Save and Load Data" setting)
// ================================================================
// Called after every operation that changes Material, Supplier,
// Warehouse stock or Product data - the same data the manual "Save
// Data" button in Data Management writes to warehouse_data.xlsx.
// Procurement Orders and Purchase Invoices already save themselves
// immediately to their own files regardless of this setting (see
// ProcurementManager/PurchaseManager); Procurement RECEIVING is the
// one exception, since receiving also changes Warehouse stock via
// Goods Receipt, so it goes through here too. A no-op, doing nothing
// extra, whenever the setting is off (the default until turned on in
// Data Management Settings).

static void autoSaveIfEnabled(
    WarehouseSystem* warehouseSystem)
{
    DataManager& dataManager =
        warehouseSystem->getDataManager();

    if (!dataManager.getAutoSaveAndLoad())
    {
        return;
    }

    dataManager.save(
        warehouseSystem->getMaterialManager(),
        warehouseSystem->getSupplierManager(),
        warehouseSystem->getWarehouseManager(),
        warehouseSystem->getProductManager());
}


// ================================================================
// CONSTRUCTOR
// ================================================================

WebServer::WebServer(
    WarehouseSystem* system)
{
    this->system = system;
}

// ============================================================
// BASE64 DECODER
// ============================================================

string decodeBase64(
    const string& encoded)
{
    static const string characters =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";


    string output;

    vector<int> table(
        256,
        -1);


    for (int i = 0;
        i < 64;
        i++)
    {
        table[
            static_cast<unsigned char>(
                characters[i])
        ] = i;
    }


    int value = 0;

    int bits = -8;


    for (unsigned char c :
    encoded)
    {
        if (table[c] == -1)
        {
            continue;
        }


        value =
            (value << 6)
            + table[c];

        bits += 6;


        if (bits >= 0)
        {
            output.push_back(
                static_cast<char>(
                    (value >> bits) & 0xFF));

            bits -= 8;
        }
    }


    return output;
}

// ================================================================
// RESOLVE ADDITIONAL SUPPLIERS (Create/Modify Material)
// ================================================================
// Reads the optional "additionalSuppliers" array from a Create/
// Modify Material request body - each entry is a non-primary
// Supplier this Material can also be purchased from. The primary
// Supplier (already resolved separately) is skipped here if it was
// also listed as an additional one, so it is never duplicated.
// Returns false with errorMessage set if any listed Supplier name
// does not exist.

bool resolveAdditionalSuppliers(
    const crow::json::rvalue& body,
    SupplierManager& supplierManager,
    const string& primarySupplierName,
    vector<MaterialSupplierLink>& out,
    string& errorMessage)
{
    out.clear();

    if (!body.has("additionalSuppliers"))
    {
        return true;
    }

    for (const auto& entry : body["additionalSuppliers"])
    {
        string supplierName =
            entry["supplier"].s();

        if (supplierName.empty() ||
            supplierName == primarySupplierName)
        {
            // Empty row from the form, or the same Supplier already
            // chosen as primary - skip instead of erroring out.
            continue;
        }

        Supplier* supplier =
            supplierManager.findSupplier(supplierName);

        if (supplier == nullptr)
        {
            errorMessage =
                "Additional Supplier not found: " + supplierName +
                ". Please create the supplier first.";

            return false;
        }

        string partNumber =
            entry.has("supplierPartNumber")
            ? string(entry["supplierPartNumber"].s())
            : "";

        MaterialSupplierLink link;

        link.supplier = supplier;
        link.supplierPartNumber = partNumber;

        out.push_back(link);
    }

    return true;
}

// ================================================================
// RUN
// ================================================================

void WebServer::run()
{
    // AuthMiddleware (AuthManager.h) checks login and role before
    // every route - see the permission rules in AuthManager::decide().

    crow::App<AuthMiddleware> app;

    auth::AuthManager::instance().load();

    WarehouseSystem* warehouseSystem = system;

    // ------------------------------------------------------------
    // Auto Save and Load Data: if turned on in Data Management
    // Settings, load Material/Supplier/Warehouse/Product data right
    // away instead of waiting for someone to click "Load Data".
    // ------------------------------------------------------------

    if (warehouseSystem->getDataManager().getAutoSaveAndLoad())
    {
        bool loaded =
            warehouseSystem->getDataManager().load(
                warehouseSystem->getMaterialManager(),
                warehouseSystem->getSupplierManager(),
                warehouseSystem->getWarehouseManager(),
                warehouseSystem->getProductManager());

        cout << (loaded ?
            "Auto Save and Load Data is on - data loaded automatically."
            : "Auto Save and Load Data is on, but no data file was found yet.")
            << endl;
    }

    // ============================================================
    // HOME - HTML
    // ============================================================

    CROW_ROUTE(app, "/")
        ([]()
            {
                string html =
                    readFile("web/index.html");

                if (html.empty())
                {
                    return crow::response(
                        500,
                        "Could not load web/index.html");
                }

                crow::response response(html);

                response.set_header(
                    "Content-Type",
                    "text/html");

                return response;
            });


    // ============================================================
    // CSS
    // ============================================================

    CROW_ROUTE(app, "/style.css")
        ([]()
            {
                string css =
                    readFile("web/style.css");

                if (css.empty())
                {
                    return crow::response(
                        404,
                        "Could not load web/style.css");
                }

                crow::response response(css);

                response.set_header(
                    "Content-Type",
                    "text/css");

                return response;
            });


    // ============================================================
    // JAVASCRIPT
    // ============================================================

    CROW_ROUTE(app, "/app.js")
        ([]()
            {
                string javascript =
                    readFile("web/app.js");

                if (javascript.empty())
                {
                    return crow::response(
                        404,
                        "Could not load web/app.js");
                }

                crow::response response(javascript);

                response.set_header(
                    "Content-Type",
                    "application/javascript");

                return response;
            });

    // ============================================================
    // LIBRARIES (quotation PDF reader + pdf.js, loaded on demand)
    // Only these listed files are served - never an arbitrary path.
    // ============================================================

    CROW_ROUTE(app, "/lib/<string>")
        ([](const string& name)
            {
                if (name != "pdf.min.js" &&
                    name != "pdf.worker.min.js" &&
                    name != "quotation-reader.js" &&
                    name != "tesseract.min.js" &&
                    name != "tesseract-worker.min.js" &&
                    name != "tesseract-core-lstm.wasm.js" &&
                    name != "tesseract-core-simd-lstm.wasm.js" &&
                    name != "eng.traineddata.gz")
                {
                    return crow::response(404, "Not found");
                }

                string content =
                    readBinaryFile("web/lib/" + name);

                if (content.empty())
                {
                    return crow::response(
                        404,
                        "Could not load web/lib/" + name);
                }

                crow::response response(content);

                const bool isData =
                    name.size() > 3 &&
                    name.compare(name.size() - 3, 3, ".gz") == 0;

                response.set_header(
                    "Content-Type",
                    isData ? "application/octet-stream" :
                             "application/javascript");

                return response;
            });

    // ============================================================
    // CREATE MATERIAL
    // ============================================================

    CROW_ROUTE(app, "/api/materials/create")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    // ------------------------------------------------
                    // Read JSON
                    // ------------------------------------------------

                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    // ------------------------------------------------
                    // Read material data
                    // ------------------------------------------------

                    string id =
                        body["id"].s();

                    // ------------------------------------------------
                    // Validate material ID
                    // ------------------------------------------------

                    if (!Material::isValidID(id))
                    {
                        return crow::response(
                            400,
                            "Invalid Material ID. Expected format ###-###### or ######-00.");
                    }

                    // ------------------------------------------------
                    // Read remaining material data
                    // ------------------------------------------------

                    string name =
                        body["name"].s();

                    string description =
                        body["description"].s();

                    string uom =
                        body["uom"].s();

                    string category =
                        body["category"].s();

                    MaterialType type =
                        Material::materialTypeFromString(
                            body["type"].s());

                    string drawingNumber =
                        body.has("drawingNumber")
                        ? string(body["drawingNumber"].s())
                        : "";

                    string drawingVersion =
                        body.has("drawingVersion")
                        ? string(body["drawingVersion"].s())
                        : "";

                    string manufacturer =
                        body.has("manufacturer")
                        ? string(body["manufacturer"].s())
                        : "";

                    string manufacturerPartNumber =
                        body.has("manufacturerPartNumber")
                        ? string(body["manufacturerPartNumber"].s())
                        : "";

                    string supplierName =
                        body["supplier"].s();

                    string supplierPartNumber =
                        body.has("supplierPartNumber")
                        ? string(body["supplierPartNumber"].s())
                        : "";

                    string photoPath =
                        body["photo"].s();

                    string photoData = "";

                    if (body.has("photoData"))
                    {
                        photoData =
                            body["photoData"].s();
                    }

                    bool active =
                        body["active"].b();

                    // ------------------------------------------------
                    // Type-dependent fields: a Design Part / PCB
                    // material is a custom-made part, identified by
                    // its technical drawing (Drawing Number + Drawing
                    // Version) rather than by a commercial part
                    // number - so it has no Manufacturer, Manufacturer
                    // Part Number or Supplier Part Number. It can
                    // still have a Supplier (the shop that makes/will
                    // make it), same as a Standard Part - see
                    // Material::requiresDrawingNumber().
                    // ------------------------------------------------

                    SupplierManager& supplierManager =
                        warehouseSystem->getSupplierManager();

                    Supplier* supplier = nullptr;

                    vector<MaterialSupplierLink> additionalSuppliers;

                    bool isCustomMade =
                        Material::requiresDrawingNumber(type);

                    if (isCustomMade)
                    {
                        if (!Material::isValidDrawingNumber(drawingNumber))
                        {
                            return crow::response(
                                400,
                                "Invalid or missing Drawing Number for this material type.");
                        }

                        if (!Material::isValidDrawingVersion(drawingVersion))
                        {
                            return crow::response(
                                400,
                                "Invalid or missing Drawing Version for this "
                                "material type. Expected a number or two "
                                "letters (e.g. AA, BA).");
                        }

                        // Design Part / PCB: no Manufacturer, no
                        // Manufacturer Part Number, and no Supplier
                        // Part Number (nothing here has a commercial
                        // part number) - but the Supplier itself is
                        // resolved below like any other material.
                        manufacturer = "";
                        manufacturerPartNumber = "";
                    }
                    else
                    {
                        // Standard Part: no drawing at all.
                        drawingNumber = "";
                        drawingVersion = "";
                    }

                    // Supplier is optional at creation for every
                    // Material type - it can be registered before a
                    // supplier has been sourced/confirmed. If a name
                    // WAS given, though, it must resolve to a real
                    // Supplier (an unresolved name is treated as a
                    // typo, not "no supplier").
                    if (!supplierName.empty())
                    {
                        supplier =
                            supplierManager.findSupplier(
                                supplierName);

                        if (supplier == nullptr)
                        {
                            return crow::response(
                                400,
                                "Supplier not found. Please create the supplier first.");
                        }

                        // ------------------------------------------
                        // Resolve additional (non-primary)
                        // Suppliers - only meaningful once there is
                        // a primary Supplier.
                        // ------------------------------------------

                        string additionalSuppliersError;

                        if (!resolveAdditionalSuppliers(
                            body,
                            supplierManager,
                            supplierName,
                            additionalSuppliers,
                            additionalSuppliersError))
                        {
                            return crow::response(
                                400,
                                additionalSuppliersError);
                        }
                    }
                    else
                    {
                        // No Supplier yet - Supplier Part Number and
                        // Additional Suppliers don't apply without a
                        // primary Supplier.
                        supplierPartNumber = "";
                    }

                    if (isCustomMade)
                    {
                        // Custom-made parts have no commercial part
                        // number at all, even once a Supplier is
                        // assigned - clear the primary Supplier Part
                        // Number and strip any that were sent along
                        // with the Additional Suppliers.
                        supplierPartNumber = "";

                        for (MaterialSupplierLink& link : additionalSuppliers)
                        {
                            link.supplierPartNumber = "";
                        }
                    }

                    // ------------------------------------------------
                    // Save image
                    // ------------------------------------------------

                    if (!photoData.empty())
                    {
                        size_t commaPosition =
                            photoData.find(',');

                        if (commaPosition != string::npos)
                        {
                            // Remove the Data URL header:
                            // data:image/jpeg;base64,
                            string encodedImage =
                                photoData.substr(
                                    commaPosition + 1);

                            // Decode Base64
                            string imageData =
                                decodeBase64(
                                    encodedImage);

                            // Create image directory
                            fs::create_directories(
                                "web/images/materials");

                            // Image path
                            photoPath =
                                "web/images/materials/"
                                + id
                                + ".jpg";

                            // Write image file
                            ofstream imageFile(
                                photoPath,
                                ios::binary);

                            if (!imageFile)
                            {
                                return crow::response(
                                    500,
                                    "Could not create image file.");
                            }

                            imageFile.write(
                                imageData.data(),
                                imageData.size());


                            imageFile.close();
                        }
                    }

                    // ------------------------------------------------
                    // Create Material
                    // ------------------------------------------------

                    Material material(
                        id,
                        name,
                        description,
                        uom,
                        category,
                        type,
                        drawingNumber,
                        drawingVersion,
                        manufacturer,
                        manufacturerPartNumber,
                        supplier,
                        supplierPartNumber,
                        photoPath,
                        active);

                    material.setAdditionalSuppliers(
                        additionalSuppliers);

                    // ------------------------------------------------
                    // Get MaterialManager
                    // ------------------------------------------------

                    MaterialManager& materialManager =
                        warehouseSystem->getMaterialManager();


                    // ------------------------------------------------
                    // Add Material
                    // ------------------------------------------------

                    bool success =
                        materialManager.createMaterial(
                            material);


                    // ------------------------------------------------
                    // Check result
                    // ------------------------------------------------

                    if (!success)
                    {
                        return crow::response(
                            409,
                            "Material ID already exists.");
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    // ------------------------------------------------
                    // Success response
                    // ------------------------------------------------

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Material created successfully.";

                    response["id"] = id;


                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // ============================================================
    // GET MATERIALS
    // ============================================================

    CROW_ROUTE(app, "/api/materials")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list materialList;


                // ------------------------------------------------
                // Get MaterialManager
                // ------------------------------------------------

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();


                // ------------------------------------------------
                // Get materials
                // ------------------------------------------------

                const vector<unique_ptr<Material>>& materials =
                    materialManager.getMaterials();


                // ------------------------------------------------
                // Convert materials to JSON
                // ------------------------------------------------

                for (const auto& material : materials)
                {
                    crow::json::wvalue item;

                    item["id"] =
                        material->getID();

                    item["name"] =
                        material->getName();

                    item["description"] =
                        material->getDescription();

                    item["uom"] =
                        material->getUoM();

                    item["category"] =
                        material->getCategory();

                    item["type"] =
                        Material::materialTypeToString(
                            material->getType());

                    item["drawingNumber"] =
                        material->getDrawingNumber();

                    item["drawingVersion"] =
                        material->getDrawingVersion();

                    item["manufacturer"] =
                        material->getManufacturer();

                    item["manufacturerPartNumber"] =
                        material->getManufacturerPartNumber();

                    item["supplier"] =
                        material->getSupplier() != nullptr
                        ? material->getSupplier()->getName()
                        : "";

                    item["supplierPartNumber"] =
                        material->getSupplierPartNumber();

                    item["photo"] =
                        material->getPhotoPath();

                    item["active"] =
                        material->isActive();


                    crow::json::wvalue::list additionalSupplierList;

                    for (const auto& link :
                        material->getAdditionalSuppliers())
                    {
                        crow::json::wvalue linkItem;

                        linkItem["supplier"] =
                            link.supplier != nullptr ?
                            link.supplier->getName() : "";

                        linkItem["supplierPartNumber"] =
                            link.supplierPartNumber;

                        additionalSupplierList.push_back(
                            linkItem);
                    }

                    item["additionalSuppliers"] =
                        std::move(additionalSupplierList);

                    materialList.push_back(
                        item);
                }


                response["materials"] =
                    std::move(materialList);


                return response;
            });

// ============================================================
// MODIFY MATERIAL
// ============================================================

    CROW_ROUTE(app, "/api/materials/modify")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    // ------------------------------------------------
                    // Read data
                    // ------------------------------------------------

                    string id =
                        body["id"].s();

                    string name =
                        body["name"].s();

                    string description =
                        body["description"].s();

                    string uom =
                        body["uom"].s();

                    string category =
                        body["category"].s();

                    MaterialType type =
                        Material::materialTypeFromString(
                            body["type"].s());

                    string drawingNumber =
                        body.has("drawingNumber")
                        ? string(body["drawingNumber"].s())
                        : "";

                    string drawingVersion =
                        body.has("drawingVersion")
                        ? string(body["drawingVersion"].s())
                        : "";

                    string manufacturer =
                        body.has("manufacturer")
                        ? string(body["manufacturer"].s())
                        : "";

                    string manufacturerPartNumber =
                        body.has("manufacturerPartNumber")
                        ? string(body["manufacturerPartNumber"].s())
                        : "";

                    string supplierName =
                        body["supplier"].s();

                    string supplierPartNumber =
                        body.has("supplierPartNumber")
                        ? string(body["supplierPartNumber"].s())
                        : "";

                    string photoPath =
                        body["photo"].s();

                    string photoData = "";

                    if (body.has("photoData"))
                    {
                        photoData =
                            body["photoData"].s();
                    }

                    bool active =
                        body["active"].b();


                    // ------------------------------------------------
                    // Validate ID
                    // ------------------------------------------------

                    if (!Material::isValidID(id))
                    {
                        return crow::response(
                            400,
                            "Invalid Material ID. Expected format ###-###### or ######-00.");
                    }

                    // ------------------------------------------------
                    // Type-dependent fields - see the same block in
                    // /api/materials/create.
                    // ------------------------------------------------

                    SupplierManager& supplierManager =
                        warehouseSystem->getSupplierManager();

                    Supplier* supplier = nullptr;

                    vector<MaterialSupplierLink> additionalSuppliers;

                    bool isCustomMade =
                        Material::requiresDrawingNumber(type);

                    if (isCustomMade)
                    {
                        if (!Material::isValidDrawingNumber(drawingNumber))
                        {
                            return crow::response(
                                400,
                                "Invalid or missing Drawing Number for this material type.");
                        }

                        if (!Material::isValidDrawingVersion(drawingVersion))
                        {
                            return crow::response(
                                400,
                                "Invalid or missing Drawing Version for this "
                                "material type. Expected a number or two "
                                "letters (e.g. AA, BA).");
                        }

                        // Design Part / PCB: no Manufacturer or
                        // Manufacturer Part Number - see the same
                        // block in /api/materials/create.
                        manufacturer = "";
                        manufacturerPartNumber = "";
                    }
                    else
                    {
                        drawingNumber = "";
                        drawingVersion = "";
                    }

                    // Supplier is optional here too, for every
                    // Material type - see the same block in
                    // /api/materials/create.
                    if (!supplierName.empty())
                    {
                        supplier =
                            supplierManager.findSupplier(
                                supplierName);

                        if (supplier == nullptr)
                        {
                            return crow::response(
                                400,
                                "Supplier not found. Please create the supplier first.");
                        }

                        string additionalSuppliersError;

                        if (!resolveAdditionalSuppliers(
                            body,
                            supplierManager,
                            supplierName,
                            additionalSuppliers,
                            additionalSuppliersError))
                        {
                            return crow::response(
                                400,
                                additionalSuppliersError);
                        }
                    }
                    else
                    {
                        supplierPartNumber = "";
                    }

                    if (isCustomMade)
                    {
                        // Custom-made parts have no commercial part
                        // number at all - see the same block in
                        // /api/materials/create.
                        supplierPartNumber = "";

                        for (MaterialSupplierLink& link : additionalSuppliers)
                        {
                            link.supplierPartNumber = "";
                        }
                    }

                    // ------------------------------------------------
                    // Save new image if provided
                    // ------------------------------------------------

                    if (!photoData.empty())
                    {
                        size_t commaPosition =
                            photoData.find(',');

                        if (commaPosition != string::npos)
                        {
                            // Remove the Data URL header

                            string encodedImage =
                                photoData.substr(
                                    commaPosition + 1);


                            // Decode Base64

                            string imageData =
                                decodeBase64(
                                    encodedImage);


                            // Create directory

                            fs::create_directories(
                                "web/images/materials");


                            // Use Material ID as image filename

                            photoPath =
                                "web/images/materials/"
                                + id
                                + ".jpg";


                            // Save image

                            ofstream imageFile(
                                photoPath,
                                ios::binary);


                            if (!imageFile)
                            {
                                return crow::response(
                                    500,
                                    "Could not create image file.");
                            }


                            imageFile.write(
                                imageData.data(),
                                imageData.size());


                            imageFile.close();
                        }
                    }

                    // ------------------------------------------------
                    // Find existing material
                    // ------------------------------------------------

                    MaterialManager& materialManager =
                        warehouseSystem->getMaterialManager();


                    Material* existingMaterial =
                        materialManager.findMaterial(id);


                    if (existingMaterial == nullptr)
                    {
                        return crow::response(
                            404,
                            "Material not found.");
                    }


                    // ------------------------------------------------
                    // Create updated data
                    // ------------------------------------------------

                    Material updatedMaterial(
                        id,
                        name,
                        description,
                        uom,
                        category,
                        type,
                        drawingNumber,
                        drawingVersion,
                        manufacturer,
                        manufacturerPartNumber,
                        supplier,
                        supplierPartNumber,
                        photoPath,
                        active);

                    updatedMaterial.setAdditionalSuppliers(
                        additionalSuppliers);


                    // ------------------------------------------------
                    // Modify
                    // ------------------------------------------------

                    bool success =
                        materialManager.modifyMaterial(
                            id,
                            updatedMaterial);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Material could not be modified.");
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Material successfully modified.";


                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // ============================================================
    // SEARCH MATERIAL
    // ============================================================

    CROW_ROUTE(app, "/api/materials/<string>")
        ([warehouseSystem](string id)
            {
                // ------------------------------------------------
                // Get MaterialManager
                // ------------------------------------------------

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();


                // ------------------------------------------------
                // Search material
                // ------------------------------------------------

                Material* material =
                    materialManager.findMaterial(id);


                // ------------------------------------------------
                // Material not found
                // ------------------------------------------------

                if (material == nullptr)
                {
                    crow::json::wvalue response;

                    response["message"] =
                        "Material not found.";

                    return crow::response(
                        404,
                        response);
                }


                // ------------------------------------------------
                // Material found
                // ------------------------------------------------

                crow::json::wvalue response;

                response["id"] =
                    material->getID();

                response["name"] =
                    material->getName();

                response["description"] =
                    material->getDescription();

                response["uom"] =
                    material->getUoM();

                response["category"] =
                    material->getCategory();

                response["type"] =
                    Material::materialTypeToString(
                        material->getType());

                response["drawingNumber"] =
                    material->getDrawingNumber();

                response["drawingVersion"] =
                    material->getDrawingVersion();

                response["manufacturer"] =
                    material->getManufacturer();

                response["manufacturerPartNumber"] =
                    material->getManufacturerPartNumber();

                response["supplier"] =
                    material->getSupplier() != nullptr
                    ? material->getSupplier()->getName()
                    : "";

                response["supplierPartNumber"] =
                    material->getSupplierPartNumber();

                response["photo"] =
                    material->getPhotoPath();

                response["active"] =
                    material->isActive();

                crow::json::wvalue::list additionalSupplierList;

                for (const auto& link :
                    material->getAdditionalSuppliers())
                {
                    crow::json::wvalue linkItem;

                    linkItem["supplier"] =
                        link.supplier != nullptr ?
                        link.supplier->getName() : "";

                    linkItem["supplierPartNumber"] =
                        link.supplierPartNumber;

                    additionalSupplierList.push_back(
                        linkItem);
                }

                response["additionalSuppliers"] =
                    std::move(additionalSupplierList);


                return crow::response(response);
            });

// ============================================================
// DELETE MATERIAL
// ============================================================

    CROW_ROUTE(app, "/api/materials/delete")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    // ------------------------------------------------
                    // Read Material ID
                    // ------------------------------------------------

                    string id =
                        body["id"].s();


                    // ------------------------------------------------
                    // Validate Material ID
                    // ------------------------------------------------

                    if (!Material::isValidID(id))
                    {
                        return crow::response(
                            400,
                            "Invalid Material ID. Expected format ###-###### or ######-00.");
                    }


                    // ------------------------------------------------
                    // Get managers
                    // ------------------------------------------------

                    MaterialManager& materialManager =
                        warehouseSystem->getMaterialManager();

                    WarehouseManager& warehouseManager =
                        warehouseSystem->getWarehouseManager();


                    // ------------------------------------------------
                    // Check if material exists
                    // ------------------------------------------------

                    Material* material =
                        materialManager.findMaterial(id);

                    if (material == nullptr)
                    {
                        return crow::response(
                            404,
                            "Material not found.");
                    }


                    // ------------------------------------------------
                    // Check if material is in use
                    // ------------------------------------------------

                    if (warehouseManager.isMaterialInUse(id))
                    {
                        return crow::response(
                            409,
                            "Material cannot be deleted because it is used in a warehouse.");
                    }


                    // ------------------------------------------------
                    // Delete material
                    // ------------------------------------------------

                    bool success =
                        materialManager.deleteMaterial(id);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Material could not be deleted.");
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    // ------------------------------------------------
                    // Response
                    // ------------------------------------------------

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Material deleted successfully.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// CREATE SUPPLIER
// ============================================================

    CROW_ROUTE(app, "/api/suppliers/create")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    // ------------------------------------------------
                    // Read supplier data
                    // ------------------------------------------------

                    string name =
                        body["name"].s();

                    if (name.empty())
                    {
                        return crow::response(
                            400,
                            "Supplier name is required.");
                    }

                    string address =
                        body.has("address")
                        ? string(body["address"].s())
                        : "";

                    string country =
                        body.has("country")
                        ? string(body["country"].s())
                        : "";

                    string contactName =
                        body.has("contactName")
                        ? string(body["contactName"].s())
                        : "";

                    string contactEmail =
                        body.has("contactEmail")
                        ? string(body["contactEmail"].s())
                        : "";

                    string website =
                        body.has("website")
                        ? string(body["website"].s())
                        : "";

                    vector<string> orderingMethods;

                    if (body.has("orderingMethods"))
                    {
                        for (const auto& method :
                            body["orderingMethods"])
                        {
                            orderingMethods.push_back(
                                method.s());
                        }
                    }

                    string paymentMethod =
                        body.has("paymentMethod")
                        ? string(body["paymentMethod"].s())
                        : "";

                    int leadTimeWeeks =
                        body.has("leadTimeWeeks")
                        ? body["leadTimeWeeks"].i()
                        : 0;

                    // An Internal Supplier is 4Tex itself
                    // (self-manufactured/custom-made parts) rather
                    // than a real external vendor - its materials are
                    // excluded from Projections and cannot be bought
                    // through a Procurement Order.

                    bool isInternal =
                        body.has("isInternal")
                        ? body["isInternal"].b()
                        : false;

                    // ------------------------------------------------
                    // Create Supplier
                    // ------------------------------------------------

                    Supplier supplier(
                        name,
                        address,
                        country,
                        contactName,
                        contactEmail,
                        website,
                        orderingMethods,
                        paymentMethod,
                        leadTimeWeeks,
                        isInternal);

                    SupplierManager& supplierManager =
                        warehouseSystem->getSupplierManager();

                    bool success =
                        supplierManager.createSupplier(
                            supplier);

                    if (!success)
                    {
                        return crow::response(
                            409,
                            "A supplier with this name already exists.");
                    }

                    autoSaveIfEnabled(warehouseSystem);

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Supplier created successfully.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// GET SUPPLIERS
// ============================================================

    CROW_ROUTE(app, "/api/suppliers")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list supplierList;

                SupplierManager& supplierManager =
                    warehouseSystem->getSupplierManager();

                const vector<unique_ptr<Supplier>>& suppliers =
                    supplierManager.getSuppliers();

                for (const auto& supplier : suppliers)
                {
                    crow::json::wvalue item;

                    item["name"] =
                        supplier->getName();

                    item["address"] =
                        supplier->getAddress();

                    item["country"] =
                        supplier->getCountry();

                    item["contactName"] =
                        supplier->getContactName();

                    item["contactEmail"] =
                        supplier->getContactEmail();

                    item["website"] =
                        supplier->getWebsite();

                    crow::json::wvalue::list orderingMethodsList;

                    for (const string& method :
                        supplier->getOrderingMethods())
                    {
                        orderingMethodsList.push_back(method);
                    }

                    item["orderingMethods"] =
                        std::move(orderingMethodsList);

                    item["paymentMethod"] =
                        supplier->getPaymentMethod();

                    item["leadTimeWeeks"] =
                        supplier->getLeadTimeWeeks();

                    item["isInternal"] =
                        supplier->getIsInternal();

                    supplierList.push_back(item);
                }

                response["suppliers"] =
                    std::move(supplierList);

                return response;
            });

// ============================================================
// MODIFY SUPPLIER
// ============================================================

    CROW_ROUTE(app, "/api/suppliers/modify")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    // ------------------------------------------------
                    // Read supplier name (unique key)
                    // ------------------------------------------------

                    string name =
                        body["name"].s();

                    if (name.empty())
                    {
                        return crow::response(
                            400,
                            "Supplier name is required.");
                    }

                    SupplierManager& supplierManager =
                        warehouseSystem->getSupplierManager();

                    if (supplierManager.findSupplier(name) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Supplier not found.");
                    }

                    // ------------------------------------------------
                    // Read remaining supplier data
                    // ------------------------------------------------

                    string address =
                        body.has("address")
                        ? string(body["address"].s())
                        : "";

                    string country =
                        body.has("country")
                        ? string(body["country"].s())
                        : "";

                    string contactName =
                        body.has("contactName")
                        ? string(body["contactName"].s())
                        : "";

                    string contactEmail =
                        body.has("contactEmail")
                        ? string(body["contactEmail"].s())
                        : "";

                    string website =
                        body.has("website")
                        ? string(body["website"].s())
                        : "";

                    vector<string> orderingMethods;

                    if (body.has("orderingMethods"))
                    {
                        for (const auto& method :
                            body["orderingMethods"])
                        {
                            orderingMethods.push_back(
                                method.s());
                        }
                    }

                    string paymentMethod =
                        body.has("paymentMethod")
                        ? string(body["paymentMethod"].s())
                        : "";

                    int leadTimeWeeks =
                        body.has("leadTimeWeeks")
                        ? body["leadTimeWeeks"].i()
                        : 0;

                    bool isInternal =
                        body.has("isInternal")
                        ? body["isInternal"].b()
                        : false;

                    // ------------------------------------------------
                    // Modify
                    // ------------------------------------------------

                    Supplier updatedSupplier(
                        name,
                        address,
                        country,
                        contactName,
                        contactEmail,
                        website,
                        orderingMethods,
                        paymentMethod,
                        leadTimeWeeks,
                        isInternal);

                    bool success =
                        supplierManager.modifySupplier(
                            name,
                            updatedSupplier);

                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Supplier could not be modified.");
                    }

                    autoSaveIfEnabled(warehouseSystem);

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Supplier successfully modified.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// SEARCH SUPPLIER
// ============================================================

    CROW_ROUTE(app, "/api/suppliers/<string>")
        ([warehouseSystem](string name)
            {
                SupplierManager& supplierManager =
                    warehouseSystem->getSupplierManager();

                Supplier* supplier =
                    supplierManager.findSupplier(name);

                if (supplier == nullptr)
                {
                    crow::json::wvalue response;

                    response["message"] =
                        "Supplier not found.";

                    return crow::response(
                        404,
                        response);
                }

                crow::json::wvalue response;

                response["name"] =
                    supplier->getName();

                response["address"] =
                    supplier->getAddress();

                response["country"] =
                    supplier->getCountry();

                response["contactName"] =
                    supplier->getContactName();

                response["contactEmail"] =
                    supplier->getContactEmail();

                response["website"] =
                    supplier->getWebsite();

                crow::json::wvalue::list orderingMethodsList;

                for (const string& method :
                    supplier->getOrderingMethods())
                {
                    orderingMethodsList.push_back(method);
                }

                response["orderingMethods"] =
                    std::move(orderingMethodsList);

                response["paymentMethod"] =
                    supplier->getPaymentMethod();

                response["leadTimeWeeks"] =
                    supplier->getLeadTimeWeeks();

                response["isInternal"] =
                    supplier->getIsInternal();

                return crow::response(response);
            });

// ============================================================
// GET SUPPLIER (lookup by name, via POST body)
// ============================================================
// Same lookup as GET /api/suppliers/<string> above, but the name
// travels in a JSON body instead of the URL path - a Supplier name
// can contain spaces, commas and periods (e.g. "Kunshan Hetex
// Trading Co., Ltd"), and putting that in a URL path segment
// depends on percent-encoding/decoding matching exactly on both
// ends, which is exactly what was failing ("Supplier not found."
// in Modify Supplier for any name with those characters). A JSON
// body sidesteps that entirely.

    CROW_ROUTE(app, "/api/suppliers/lookup")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                auto body =
                    crow::json::load(req.body);

                if (!body || !body.has("name"))
                {
                    crow::json::wvalue response;

                    response["message"] =
                        "Supplier name is required.";

                    return crow::response(
                        400,
                        response);
                }

                string name =
                    body["name"].s();

                SupplierManager& supplierManager =
                    warehouseSystem->getSupplierManager();

                Supplier* supplier =
                    supplierManager.findSupplier(name);

                if (supplier == nullptr)
                {
                    crow::json::wvalue response;

                    response["message"] =
                        "Supplier not found.";

                    return crow::response(
                        404,
                        response);
                }

                crow::json::wvalue response;

                response["name"] =
                    supplier->getName();

                response["address"] =
                    supplier->getAddress();

                response["country"] =
                    supplier->getCountry();

                response["contactName"] =
                    supplier->getContactName();

                response["contactEmail"] =
                    supplier->getContactEmail();

                response["website"] =
                    supplier->getWebsite();

                crow::json::wvalue::list orderingMethodsList;

                for (const string& method :
                    supplier->getOrderingMethods())
                {
                    orderingMethodsList.push_back(method);
                }

                response["orderingMethods"] =
                    std::move(orderingMethodsList);

                response["paymentMethod"] =
                    supplier->getPaymentMethod();

                response["leadTimeWeeks"] =
                    supplier->getLeadTimeWeeks();

                response["isInternal"] =
                    supplier->getIsInternal();

                return crow::response(response);
            });

// ============================================================
// DELETE SUPPLIER
// ============================================================

    CROW_ROUTE(app, "/api/suppliers/delete")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string name =
                        body["name"].s();

                    SupplierManager& supplierManager =
                        warehouseSystem->getSupplierManager();

                    MaterialManager& materialManager =
                        warehouseSystem->getMaterialManager();

                    if (supplierManager.findSupplier(name) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Supplier not found.");
                    }

                    // ------------------------------------------------
                    // Prevent deleting a supplier still referenced
                    // by a material.
                    // ------------------------------------------------

                    if (materialManager.isSupplierInUse(name))
                    {
                        return crow::response(
                            409,
                            "Supplier cannot be deleted because it is used by a material.");
                    }

                    bool success =
                        supplierManager.deleteSupplier(name);

                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Supplier could not be deleted.");
                    }

                    autoSaveIfEnabled(warehouseSystem);

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Supplier deleted successfully.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// SAVE DATA
// ============================================================

    CROW_ROUTE(app, "/api/data/save")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem]()
            {
                bool success =
                    warehouseSystem->getDataManager()
                    .save(
                        warehouseSystem->getMaterialManager(),
                        warehouseSystem->getSupplierManager(),
                        warehouseSystem->getWarehouseManager(),
                        warehouseSystem->getProductManager());


                if (!success)
                {
                    string reason =
                        warehouseSystem->getDataManager()
                        .getLastSaveError();

                    return crow::response(
                        reason.empty() ? 500 : 409,
                        reason.empty() ?
                        "Error saving data." : reason);
                }


                return crow::response(
                    200,
                    "Data saved successfully.");
            });

// ============================================================
// EXIT APPLICATION
// ============================================================
// Used by the "Exit" button on the main menu, so the user does not
// have to go back to Visual Studio / the console window to stop the
// server. Saves everything first (the same save the "Save Data"
// button performs), then shuts the server down a moment later - the
// short delay lets this response actually reach the browser before
// the process exits, instead of the connection dropping mid-request.
// app.stop() makes app.port(...).run() (see the bottom of this file)
// return normally, so control flows back to main() and the program
// ends on its own "return 0;" rather than being killed abruptly.

    CROW_ROUTE(app, "/api/data/exit")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem, &app]()
            {
                bool saved =
                    warehouseSystem->getDataManager()
                    .save(
                        warehouseSystem->getMaterialManager(),
                        warehouseSystem->getSupplierManager(),
                        warehouseSystem->getWarehouseManager(),
                        warehouseSystem->getProductManager());

                thread([&app]()
                    {
                        this_thread::sleep_for(
                            chrono::milliseconds(300));

                        app.stop();
                    }).detach();

                crow::json::wvalue response;

                response["success"] = true;

                response["dataSaved"] = saved;

                response["message"] =
                    saved ?
                    "Data saved. Shutting down - you can close this tab." :
                    "Could not save data, but shutting down anyway - "
                    "you can close this tab.";

                return crow::response(response);
            });

// ============================================================
// LOAD DATA
// ============================================================

    CROW_ROUTE(app, "/api/data/load")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem]()
            {
                bool success =
                    warehouseSystem->getDataManager()
                    .load(
                        warehouseSystem->getMaterialManager(),
                        warehouseSystem->getSupplierManager(),
                        warehouseSystem->getWarehouseManager(),
                        warehouseSystem->getProductManager());


                if (!success)
                {
                    return crow::response(
                        500,
                        "Error loading data.");
                }


                return crow::response(
                    200,
                    "Data loaded successfully.");
            });

// ============================================================
// DATA MANAGEMENT - AUTO SAVE AND LOAD CONFIG (GET)
// ============================================================

    CROW_ROUTE(app, "/api/data/config")
        .methods(crow::HTTPMethod::GET)
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                response["autoSaveAndLoad"] =
                    warehouseSystem->getDataManager()
                        .getAutoSaveAndLoad();

                return response;
            });

// ============================================================
// DATA MANAGEMENT - AUTO SAVE AND LOAD CONFIG (SET)
// ============================================================

    CROW_ROUTE(app, "/api/data/config")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    bool enabled =
                        body["autoSaveAndLoad"].b();

                    warehouseSystem->getDataManager()
                        .setAutoSaveAndLoad(enabled);

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["autoSaveAndLoad"] = enabled;

                    response["message"] =
                        enabled ?
                        "Auto Save and Load Data turned on." :
                        "Auto Save and Load Data turned off.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// DATA MANAGEMENT - DOCUMENTS FOLDER (GET)
// ============================================================
// The root folder (an absolute path the user chooses, e.g. a synced
// folder) under which Procurement documents (Order Confirmation /
// Lieferschein PDFs) get saved - see the upload routes below.

    CROW_ROUTE(app, "/api/settings/documents-folder")
        .methods(crow::HTTPMethod::GET)
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                response["documentsFolder"] =
                    warehouseSystem->getDataManager()
                        .getDocumentsFolder();

                return response;
            });

// ============================================================
// DATA MANAGEMENT - DOCUMENTS FOLDER (SET)
// ============================================================

    CROW_ROUTE(app, "/api/settings/documents-folder")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body || !body.has("documentsFolder"))
                    {
                        return crow::response(
                            400,
                            "documentsFolder is required.");
                    }

                    string folder =
                        body["documentsFolder"].s();

                    // Validate it by actually creating it (or
                    // confirming it already exists) right now,
                    // instead of discovering later - at upload time -
                    // that the path was invalid or not writable.

                    if (!folder.empty())
                    {
                        try
                        {
                            fs::create_directories(folder);
                        }
                        catch (const exception&)
                        {
                            return crow::response(
                                400,
                                "Could not create or access that folder. "
                                "Check the path and try again.");
                        }
                    }

                    warehouseSystem->getDataManager()
                        .setDocumentsFolder(folder);

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["documentsFolder"] = folder;

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// DATA MANAGEMENT - RESET PRC / INVOICE NUMBERING
// ============================================================
// Wipes every Procurement Order and every Purchase Invoice (and with
// it, all Material price history, since that is derived from
// invoices) and restarts both "PRC-######" and "INV-######"
// numbering at 1. Meant for clearing out test data before real use
// begins - not for everyday use, and there is no undo.

    CROW_ROUTE(app, "/api/data/reset-numbering")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body || !body.has("confirm") ||
                        string(body["confirm"].s()) != "RESET")
                    {
                        return crow::response(
                            400,
                            "Type RESET to confirm - this permanently "
                            "deletes every Procurement Order, Purchase "
                            "Invoice and Production Projection, and "
                            "clears the Movement Log. Material, "
                            "Supplier, Warehouse and Product data "
                            "(warehouse_data.xlsx) and every setting "
                            "are not touched.");
                    }

                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();

                    PurchaseManager& purchaseManager =
                        warehouseSystem->getPurchaseManager();

                    ProjectionManager& projectionManager =
                        warehouseSystem->getProjectionManager();

                    procurementManager.clear();
                    procurementManager.save();

                    purchaseManager.clear();
                    purchaseManager.save();

                    projectionManager.clear();
                    projectionManager.save();

                    MovementLogger& movementLogger =
                        warehouseSystem->getMovementLogger();

                    // Written directly rather than through
                    // logSystemEvent() (which no-ops when the
                    // "log data operations" setting is off) and after
                    // clearHistory() wipes the file, so this reset is
                    // always the one line left behind, on or off.
                    movementLogger.clearHistory();

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "All Procurement Orders, Purchase Invoices and "
                        "Production Projections were deleted, and the "
                        "Movement Log was cleared. PRC and INV "
                        "numbering now starts at 1. Material, Supplier, "
                        "Warehouse and Product data was not touched.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });


// ============================================================
// CREATE WAREHOUSE
// ============================================================

    CROW_ROUTE(app, "/api/warehouses/create")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    int id =
                        body["id"].i();

                    string name =
                        body["name"].s();


                    if (name.empty())
                    {
                        return crow::response(
                            400,
                            "Warehouse name is required.");
                    }


                    WarehouseManager& warehouseManager =
                        warehouseSystem->getWarehouseManager();


                    bool success =
                        warehouseManager.createWarehouse(
                            id,
                            name);


                    if (!success)
                    {
                        return crow::response(
                            409,
                            "Warehouse ID already exists.");
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Warehouse created successfully.";

                    response["id"] = id;

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// GET WAREHOUSES
// ============================================================

    CROW_ROUTE(app, "/api/warehouses")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list warehouseList;


                WarehouseManager& warehouseManager =
                    warehouseSystem->getWarehouseManager();


                vector<Warehouse*>& warehouses =
                    warehouseManager.getWarehouses();


                for (const Warehouse* warehouse : warehouses)
                {
                    crow::json::wvalue item;

                    item["id"] =
                        warehouse->getID();

                    item["name"] =
                        warehouse->getName();


                    warehouseList.push_back(
                        item);
                }


                response["warehouses"] =
                    std::move(warehouseList);


                return response;
            });

// ============================================================
// DELETE WAREHOUSE
// ============================================================

    CROW_ROUTE(app, "/api/warehouses/delete")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    int id =
                        body["id"].i();


                    WarehouseManager& warehouseManager =
                        warehouseSystem->getWarehouseManager();


                    Warehouse* warehouse =
                        warehouseManager.findWarehouse(id);


                    if (warehouse == nullptr)
                    {
                        return crow::response(
                            404,
                            "Warehouse not found.");
                    }

                    // ------------------------------------------------
                    // Check if warehouse is empty
                    // ------------------------------------------------

                    {
                        string lockedError =
                            warehouseLockedError(
                                warehouseSystem,
                                id);

                        if (!lockedError.empty())
                        {
                            return crow::response(
                                409,
                                lockedError);
                        }
                    }

                    if (!warehouse->isEmpty())
                    {
                        return crow::response(
                            409,
                            "Warehouse cannot be deleted because it contains inventory.");
                    }

                    // ------------------------------------------------
                    // Delete warehouse
                    // ------------------------------------------------

                    bool success =
                        warehouseManager.deleteWarehouse(id);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Warehouse could not be deleted.");
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Warehouse deleted successfully.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// GOODS RECEIPT
// ============================================================

    CROW_ROUTE(app, "/api/inventory/receipt")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    // ------------------------------------------------
                    // Read data
                    // ------------------------------------------------

                    int warehouseID =
                        body["warehouseID"].i();

                    string materialID =
                        body["materialID"].s();

                    double quantity =
                        jsonQuantity(body["quantity"]);

                    {
                        string decimalsError =
                            quantityDecimalsError(quantity, "Quantity");

                        if (!decimalsError.empty())
                        {
                            return crow::response(
                                400,
                                decimalsError);
                        }
                    }

                    string comment = "";

                    if (body.has("comment"))
                    {
                        comment =
                            body["comment"].s();
                    }

                    // ------------------------------------------------
                    // Validate
                    // ------------------------------------------------

                    if (!Material::isValidID(materialID))
                    {
                        return crow::response(
                            400,
                            "Invalid Material ID. Expected format ###-###### or ######-00.");
                    }


                    if (quantity <= 0)
                    {
                        return crow::response(
                            400,
                            "Quantity must be greater than zero.");
                    }


                    // ------------------------------------------------
                    // Find Material
                    // ------------------------------------------------

                    MaterialManager& materialManager =
                        warehouseSystem->getMaterialManager();


                    Material* material =
                        materialManager.findMaterial(
                            materialID);


                    if (material == nullptr)
                    {
                        return crow::response(
                            404,
                            "Material not found.");
                    }


                    // ------------------------------------------------
                    // Goods Receipt
                    // ------------------------------------------------

                    {
                        string lockedError =
                            warehouseLockedError(
                                warehouseSystem,
                                warehouseID);

                        if (!lockedError.empty())
                        {
                            return crow::response(
                                409,
                                lockedError);
                        }
                    }

                    InventoryManager& inventoryManager =
                        warehouseSystem->getInventoryManager();


                    bool success =
                        inventoryManager.goodsReceipt(
                            warehouseID,
                            material,
                            quantity,
                            comment);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Goods receipt failed.");
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    // ------------------------------------------------
                    // Response
                    // ------------------------------------------------

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Goods receipt completed successfully.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });


// ============================================================
// GOODS ISSUE
// ============================================================

    CROW_ROUTE(app, "/api/inventory/issue")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    // ------------------------------------------------
                    // Read data
                    // ------------------------------------------------

                    int warehouseID =
                        body["warehouseID"].i();

                    string materialID =
                        body["materialID"].s();

                    double quantity =
                        jsonQuantity(body["quantity"]);

                    {
                        string decimalsError =
                            quantityDecimalsError(quantity, "Quantity");

                        if (!decimalsError.empty())
                        {
                            return crow::response(
                                400,
                                decimalsError);
                        }
                    }

                    string comment = "";

                    if (body.has("comment"))
                    {
                        comment =
                            body["comment"].s();
                    }

                    // ------------------------------------------------
                    // Validate
                    // ------------------------------------------------

                    if (!Material::isValidID(
                        materialID))
                    {
                        return crow::response(
                            400,
                            "Invalid Material ID. Expected format ###-###### or ######-00.");
                    }


                    if (quantity <= 0)
                    {
                        return crow::response(
                            400,
                            "Quantity must be greater than zero.");
                    }


                    // ------------------------------------------------
                    // Find Warehouse
                    // ------------------------------------------------

                    WarehouseManager& warehouseManager =
                        warehouseSystem->getWarehouseManager();


                    Warehouse* warehouse =
                        warehouseManager.findWarehouse(
                            warehouseID);


                    if (warehouse == nullptr)
                    {
                        return crow::response(
                            404,
                            "Warehouse not found.");
                    }


                    // ------------------------------------------------
                    // Check material in warehouse
                    // ------------------------------------------------

                    WarehouseNode* node =
                        warehouse->findMaterial(
                            materialID);


                    if (node == nullptr)
                    {
                        return crow::response(
                            404,
                            "Material not found in warehouse.");
                    }


                    // ------------------------------------------------
                    // Check available quantity
                    // ------------------------------------------------

                    if (qtyLess(node->quantity, quantity))
                    {
                        return crow::response(
                            409,
                            "Insufficient quantity available.");
                    }


                    // ------------------------------------------------
                    // Execute Goods Issue
                    // ------------------------------------------------

                    {
                        string lockedError =
                            warehouseLockedError(
                                warehouseSystem,
                                warehouseID);

                        if (!lockedError.empty())
                        {
                            return crow::response(
                                409,
                                lockedError);
                        }
                    }

                    InventoryManager& inventoryManager =
                        warehouseSystem->getInventoryManager();


                    bool success =
                        inventoryManager.goodsIssue(
                            warehouseID,
                            materialID,
                            quantity,
                            comment);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Goods issue failed.");
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    // ------------------------------------------------
                    // Response
                    // ------------------------------------------------

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Goods issue completed successfully.";


                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // ============================================================
    // TRANSFER MATERIAL
    // ============================================================

    CROW_ROUTE(app, "/api/inventory/transfer")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    // ------------------------------------------------
                    // Read data
                    // ------------------------------------------------

                    int sourceWarehouseID =
                        body["sourceWarehouseID"].i();

                    int destinationWarehouseID =
                        body["destinationWarehouseID"].i();

                    string materialID =
                        body["materialID"].s();

                    double quantity =
                        jsonQuantity(body["quantity"]);

                    {
                        string decimalsError =
                            quantityDecimalsError(quantity, "Quantity");

                        if (!decimalsError.empty())
                        {
                            return crow::response(
                                400,
                                decimalsError);
                        }
                    }

                    string comment = "";

                    if (body.has("comment"))
                    {
                        comment =
                            body["comment"].s();
                    }


                    // ------------------------------------------------
                    // Validation
                    // ------------------------------------------------

                    if (sourceWarehouseID ==
                        destinationWarehouseID)
                    {
                        return crow::response(
                            400,
                            "Source and destination warehouses must be different.");
                    }


                    if (!Material::isValidID(materialID))
                    {
                        return crow::response(
                            400,
                            "Invalid Material ID. Expected format ###-###### or ######-00.");
                    }


                    if (quantity <= 0)
                    {
                        return crow::response(
                            400,
                            "Quantity must be greater than zero.");
                    }


                    // ------------------------------------------------
                    // Execute transfer
                    // ------------------------------------------------

                    {
                        string lockedError =
                            warehouseLockedError(
                                warehouseSystem,
                                sourceWarehouseID);

                        if (!lockedError.empty())
                        {
                            return crow::response(
                                409,
                                lockedError);
                        }
                    }

                    {
                        string lockedError =
                            warehouseLockedError(
                                warehouseSystem,
                                destinationWarehouseID);

                        if (!lockedError.empty())
                        {
                            return crow::response(
                                409,
                                lockedError);
                        }
                    }

                    InventoryManager& inventoryManager =
                        warehouseSystem->getInventoryManager();


                    bool success =
                        inventoryManager.transferMaterial(
                            sourceWarehouseID,
                            destinationWarehouseID,
                            materialID,
                            quantity,
                            comment);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Transfer failed. Check warehouses, material, and available quantity.");
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    // ------------------------------------------------
                    // Response
                    // ------------------------------------------------

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Material transferred successfully.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // ============================================================
    // GET INVENTORY
    // ============================================================

    CROW_ROUTE(app, "/api/inventory")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list warehouseList;


                WarehouseManager& warehouseManager =
                    warehouseSystem->getWarehouseManager();


                vector<Warehouse*>& warehouses =
                    warehouseManager.getWarehouses();


                for (const Warehouse* warehouse : warehouses)
                {
                    crow::json::wvalue warehouseItem;

                    warehouseItem["id"] =
                        warehouse->getID();

                    warehouseItem["name"] =
                        warehouse->getName();


                    crow::json::wvalue::list inventoryList;


                    // ------------------------------------------------
                    // Read Warehouse linked list
                    // ------------------------------------------------

                    WarehouseNode* node =
                        warehouse->getHead();


                    while (node != nullptr)
                    {
                        crow::json::wvalue inventoryItem;

                        inventoryItem["materialID"] =
                            node->material->getID();

                        inventoryItem["materialName"] =
                            node->material->getName();

                        inventoryItem["quantity"] =
                            node->quantity;

                        inventoryItem["uom"] =
                            node->material->getUoM();


                        inventoryList.push_back(
                            inventoryItem);


                        node = node->next;
                    }


                    warehouseItem["inventory"] =
                        std::move(inventoryList);


                    warehouseList.push_back(
                        warehouseItem);
                }


                response["warehouses"] =
                    std::move(warehouseList);


                return response;
            });

    // ============================================================
    // INVENTORY CHECK
    // ============================================================

    CROW_ROUTE(app, "/api/inventory/check")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list problems;


                WarehouseManager& warehouseManager =
                    warehouseSystem->getWarehouseManager();


                vector<Warehouse*>& warehouses =
                    warehouseManager.getWarehouses();


                int entriesChecked = 0;

                bool valid = true;


                // ------------------------------------------------
                // Check every warehouse
                // ------------------------------------------------

                for (const Warehouse* warehouse : warehouses)
                {
                    if (warehouse == nullptr)
                    {
                        valid = false;

                        problems.push_back(
                            "Null warehouse pointer found.");

                        continue;
                    }


                    // --------------------------------------------
                    // Check inventory nodes
                    // --------------------------------------------

                    WarehouseNode* node =
                        warehouse->getHead();


                    while (node != nullptr)
                    {
                        entriesChecked++;


                        // ----------------------------------------
                        // Check Material pointer
                        // ----------------------------------------

                        if (node->material == nullptr)
                        {
                            valid = false;

                            problems.push_back(
                                "Warehouse " +
                                to_string(
                                    warehouse->getID()) +
                                " contains an invalid material pointer.");

                            node = node->next;

                            continue;
                        }


                        // ----------------------------------------
                        // Check quantity
                        // ----------------------------------------

                        if (node->quantity <= 0)
                        {
                            valid = false;

                            problems.push_back(
                                "Warehouse " +
                                to_string(
                                    warehouse->getID()) +
                                " contains material " +
                                node->material->getID() +
                                " with invalid quantity.");
                        }


                        // ----------------------------------------
                        // Check Material ID
                        // ----------------------------------------

                        if (!Material::isValidID(
                            node->material->getID()))
                        {
                            valid = false;

                            problems.push_back(
                                "Warehouse " +
                                to_string(
                                    warehouse->getID()) +
                                " contains material with invalid ID: " +
                                node->material->getID());
                        }


                        node = node->next;
                    }
                }


                // ------------------------------------------------
                // Response
                // ------------------------------------------------

                response["valid"] =
                    valid;

                response["warehousesChecked"] =
                    static_cast<int>(
                        warehouses.size());

                response["entriesChecked"] =
                    entriesChecked;

                response["problems"] =
                    std::move(problems);


                return response;
            });


// ============================================================
// CREATE PRODUCT
// ============================================================

    CROW_ROUTE(app, "/api/products/create")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    string id =
                        body["id"].s();

                    string name =
                        body["name"].s();

                    string description =
                        body["description"].s();

                    // Optional - 0 means "none set", same as a
                    // Product that never had one assigned.
                    int mainWarehouseID =
                        body.has("mainWarehouseID") ?
                        body["mainWarehouseID"].i() : 0;


                    // ------------------------------------------------
                    // Validate Product ID
                    // ------------------------------------------------

                    if (!Material::isValidID(id))
                    {
                        return crow::response(
                            400,
                            "Invalid Product ID. Expected format ###-######.");
                    }


                    if (name.empty())
                    {
                        return crow::response(
                            400,
                            "Product Name is required.");
                    }


                    if (mainWarehouseID != 0 &&
                        warehouseSystem->getWarehouseManager()
                            .findWarehouse(mainWarehouseID) == nullptr)
                    {
                        return crow::response(
                            400,
                            "Main Warehouse not found.");
                    }


                    // ------------------------------------------------
                    // Create Product
                    // ------------------------------------------------

                    Product product(
                        id,
                        name,
                        description,
                        mainWarehouseID);


                    // ------------------------------------------------
                    // Read BOM
                    // ------------------------------------------------

                    if (body.has("bom"))
                    {
                        for (
                            const auto& item :
                            body["bom"]
                            )
                        {
                            string materialID =
                                item["materialID"].s();

                            double quantity =
                                jsonQuantity(item["quantity"]);

                            {
                                string decimalsError =
                                    quantityDecimalsError(quantity, "BOM quantity");

                                if (!decimalsError.empty())
                                {
                                    return crow::response(
                                        400,
                                        decimalsError);
                                }
                            }


                            if (!Material::isValidID(
                                materialID))
                            {
                                return crow::response(
                                    400,
                                    "Invalid Material ID in BOM.");
                            }


                            if (quantity <= 0)
                            {
                                return crow::response(
                                    400,
                                    "BOM quantity must be greater than zero.");
                            }


                            if (!product.addBOMItem(
                                materialID,
                                quantity))
                            {
                                return crow::response(
                                    400,
                                    "Could not add BOM item.");
                            }
                        }
                    }


                    ProductManager& productManager =
                        warehouseSystem->getProductManager();


                    if (!productManager.createProduct(
                        product))
                    {
                        return crow::response(
                            409,
                            "Product already exists or a BOM material was not found.");
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Product created successfully.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// GET PRODUCT (lookup by ID, for Modify Product)
// ============================================================

    CROW_ROUTE(app, "/api/products/<string>")
        ([warehouseSystem](string id)
            {
                ProductManager& productManager =
                    warehouseSystem->getProductManager();

                Product* product =
                    productManager.findProduct(id);

                if (product == nullptr)
                {
                    crow::json::wvalue response;

                    response["message"] =
                        "Product not found.";

                    return crow::response(
                        404,
                        response);
                }

                crow::json::wvalue response;

                response["id"] =
                    product->getID();

                response["name"] =
                    product->getName();

                response["description"] =
                    product->getDescription();

                response["mainWarehouseID"] =
                    product->getMainWarehouseID();

                crow::json::wvalue::list bomList;

                for (const auto& bomItem :
                    product->getBOM())
                {
                    crow::json::wvalue bom;

                    bom["materialID"] =
                        bomItem.materialID;

                    bom["quantity"] =
                        bomItem.quantity;

                    bomList.push_back(
                        std::move(bom));
                }

                response["bom"] =
                    std::move(bomList);

                return crow::response(response);
            });

// ============================================================
// MODIFY PRODUCT
// ============================================================

    CROW_ROUTE(app, "/api/products/modify")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string id =
                        body["id"].s();

                    string name =
                        body["name"].s();

                    string description =
                        body.has("description") ?
                        string(body["description"].s()) : "";


                    ProductManager& productManager =
                        warehouseSystem->getProductManager();

                    Product* existingProduct =
                        productManager.findProduct(id);

                    if (existingProduct == nullptr)
                    {
                        return crow::response(
                            404,
                            "Product not found.");
                    }

                    if (name.empty())
                    {
                        return crow::response(
                            400,
                            "Product Name is required.");
                    }

                    // Optional - if the request does not send it at
                    // all, keep whatever the Product already has
                    // rather than silently clearing it back to "none
                    // set". Sending 0 explicitly does clear it.
                    int mainWarehouseID =
                        body.has("mainWarehouseID") ?
                        body["mainWarehouseID"].i() :
                        existingProduct->getMainWarehouseID();

                    if (mainWarehouseID != 0 &&
                        warehouseSystem->getWarehouseManager()
                            .findWarehouse(mainWarehouseID) == nullptr)
                    {
                        return crow::response(
                            400,
                            "Main Warehouse not found.");
                    }


                    // ------------------------------------------------
                    // Read BOM
                    // ------------------------------------------------

                    vector<BOMItem> bom;

                    MaterialManager& materialManager =
                        warehouseSystem->getMaterialManager();

                    if (body.has("bom"))
                    {
                        for (
                            const auto& item :
                            body["bom"]
                            )
                        {
                            string materialID =
                                item["materialID"].s();

                            double quantity =
                                jsonQuantity(item["quantity"]);

                            {
                                string decimalsError =
                                    quantityDecimalsError(quantity, "BOM quantity");

                                if (!decimalsError.empty())
                                {
                                    return crow::response(
                                        400,
                                        decimalsError);
                                }
                            }


                            if (!Material::isValidID(
                                materialID))
                            {
                                return crow::response(
                                    400,
                                    "Invalid Material ID in BOM.");
                            }


                            if (materialManager.findMaterial(
                                materialID) == nullptr)
                            {
                                return crow::response(
                                    404,
                                    "Material not found in BOM: " +
                                    materialID);
                            }


                            if (quantity <= 0)
                            {
                                return crow::response(
                                    400,
                                    "BOM quantity must be greater than zero.");
                            }

                            BOMItem bomItem;

                            bomItem.materialID = materialID;
                            bomItem.quantity = roundQty(quantity);

                            bom.push_back(bomItem);
                        }
                    }


                    if (!productManager.modifyProduct(
                        id,
                        name,
                        description,
                        bom,
                        mainWarehouseID))
                    {
                        return crow::response(
                            400,
                            "Could not modify Product.");
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Product modified successfully.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// GET PRODUCTS
// ============================================================

    CROW_ROUTE(app, "/api/products")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list productList;


                ProductManager& productManager =
                    warehouseSystem->getProductManager();

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();

                WarehouseManager& warehouseManager =
                    warehouseSystem->getWarehouseManager();

                vector<Warehouse*>& warehouses =
                    warehouseManager.getWarehouses();


                const auto& products =
                    productManager.getProducts();


                for (const auto& product :
                    products)
                {
                    crow::json::wvalue item;

                    item["id"] =
                        product->getID();

                    item["name"] =
                        product->getName();

                    item["description"] =
                        product->getDescription();

                    int mainWarehouseID =
                        product->getMainWarehouseID();

                    item["mainWarehouseID"] =
                        mainWarehouseID;

                    // Looked up here once per product rather than
                    // making the frontend match IDs against the
                    // separate /api/warehouses list.
                    Warehouse* mainWarehouse = nullptr;

                    if (mainWarehouseID != 0)
                    {
                        for (Warehouse* warehouse : warehouses)
                        {
                            if (warehouse->getID() == mainWarehouseID)
                            {
                                mainWarehouse = warehouse;
                                break;
                            }
                        }
                    }

                    item["mainWarehouseName"] =
                        (mainWarehouse != nullptr) ?
                        mainWarehouse->getName() : "";


                    crow::json::wvalue::list bomList;


                    for (const auto& bomItem :
                        product->getBOM())
                    {
                        crow::json::wvalue bom;

                        bom["materialID"] =
                            bomItem.materialID;

                        Material* material =
                            materialManager.findMaterial(
                                bomItem.materialID);

                        bom["materialName"] =
                            (material != nullptr) ?
                            material->getName() : "";

                        bom["drawingNumber"] =
                            (material != nullptr) ?
                            material->getDrawingNumber() : "";

                        bom["photo"] =
                            (material != nullptr) ?
                            material->getPhotoPath() : "";

                        bom["quantity"] =
                            bomItem.quantity;

                        // Internal Supplier materials are self-
                        // manufactured, not purchased or stocked in a
                        // Warehouse the same way (see Supplier
                        // Management) - the frontend greys this row
                        // out and skips the stock column entirely
                        // instead of showing a misleading "0".
                        bom["hasInternalSupplier"] =
                            (material != nullptr) &&
                            material->hasInternalSupplier();


                        // ------------------------------------------------
                        // Stock of this material in each Warehouse, so the
                        // UI can flag whether there is enough on hand to
                        // build 10 units of this Product (10 * bomItem's
                        // per-unit quantity, per Warehouse). Once a
                        // Product has a Main Warehouse set, only that
                        // Warehouse's stock is worth showing - every other
                        // Warehouse is almost always irrelevant/zero for
                        // this Product, and just adds noise. A Product
                        // that has never had a Main Warehouse assigned
                        // (mainWarehouse == nullptr) falls back to the
                        // original behavior of listing every Warehouse.

                        crow::json::wvalue::list warehouseStockList;

                        for (const Warehouse* warehouse : warehouses)
                        {
                            if (mainWarehouse != nullptr &&
                                warehouse != mainWarehouse)
                            {
                                continue;
                            }

                            crow::json::wvalue stockItem;

                            stockItem["warehouseID"] =
                                warehouse->getID();

                            stockItem["warehouseName"] =
                                warehouse->getName();


                            WarehouseNode* node =
                                warehouse->findMaterial(
                                    bomItem.materialID);

                            double quantity =
                                (node != nullptr) ?
                                node->quantity : 0.0;

                            stockItem["quantity"] =
                                quantity;

                            stockItem["sufficientForSafetyStock"] =
                                !qtyLess(quantity, bomItem.quantity *
                                    productManager.getSafetyStockUnits());


                            warehouseStockList.push_back(
                                std::move(stockItem));
                        }


                        bom["warehouseStock"] =
                            std::move(warehouseStockList);


                        bomList.push_back(
                            std::move(bom));
                    }


                    item["bom"] =
                        std::move(bomList);


                    productList.push_back(
                        std::move(item));
                }


                response["products"] =
                    std::move(productList);

                response["safetyStockUnits"] =
                    productManager.getSafetyStockUnits();


                return response;
            });

// ============================================================
// PRODUCT - SAFETY STOCK CONFIG (GET)
// ============================================================

    CROW_ROUTE(app, "/api/products/config")
        .methods(crow::HTTPMethod::GET)
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                response["safetyStockUnits"] =
                    warehouseSystem->getProductManager()
                        .getSafetyStockUnits();

                return response;
            });

// ============================================================
// PRODUCT - SAFETY STOCK CONFIG (SET)
// ============================================================

    CROW_ROUTE(app, "/api/products/config")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    int safetyStockUnits =
                        body["safetyStockUnits"].i();

                    ProductManager& productManager =
                        warehouseSystem->getProductManager();

                    bool success =
                        productManager.setSafetyStockUnits(
                            safetyStockUnits);

                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Safety Stock must be greater than zero.");
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["safetyStockUnits"] =
                        safetyStockUnits;

                    response["message"] =
                        "Safety Stock updated.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// DELETE PRODUCT
// ============================================================

    CROW_ROUTE(app, "/api/products/delete")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    string id =
                        body["id"].s();


                    if (!Material::isValidID(id))
                    {
                        return crow::response(
                            400,
                            "Invalid Product ID. Expected format ###-######.");
                    }


                    ProductManager& productManager =
                        warehouseSystem->getProductManager();


                    Product* product =
                        productManager.findProduct(id);


                    if (product == nullptr)
                    {
                        return crow::response(
                            404,
                            "Product not found.");
                    }


                    bool success =
                        productManager.deleteProduct(id);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Product could not be deleted.");
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Product deleted successfully.";


                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// SELL PRODUCT
// ============================================================

    CROW_ROUTE(app, "/api/products/sell")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    // ------------------------------------------------
                    // Read data
                    // ------------------------------------------------

                    int warehouseID =
                        body["warehouseID"].i();

                    string productID =
                        body["productID"].s();

                    // Product-level count (units sold): whole numbers only
                    int quantity =
                        body["quantity"].i();

                    // Optional note identifying this sale (customer,
                    // order number...) - goes into the movement ledger
                    string comment = "";

                    if (body.has("comment"))
                    {
                        comment = body["comment"].s();
                    }


                    // ------------------------------------------------
                    // Validate
                    // ------------------------------------------------

                    if (!Material::isValidID(productID))
                    {
                        return crow::response(
                            400,
                            "Invalid Product ID. Expected format ###-######.");
                    }


                    if (quantity <= 0)
                    {
                        return crow::response(
                            400,
                            "Quantity must be greater than zero.");
                    }


                    // ------------------------------------------------
                    // Execute sale
                    // ------------------------------------------------

                    {
                        string lockedError =
                            warehouseLockedError(
                                warehouseSystem,
                                warehouseID);

                        if (!lockedError.empty())
                        {
                            return crow::response(
                                409,
                                lockedError);
                        }
                    }

                    ProductManager& productManager =
                        warehouseSystem->getProductManager();


                    InventoryManager& inventoryManager =
                        warehouseSystem->getInventoryManager();


                    bool success =
                        inventoryManager.sellProduct(
                            productManager,
                            warehouseID,
                            productID,
                            quantity,
                            comment);


                    if (!success)
                    {
                        return crow::response(
                            409,
                            inventoryManager.getLastError().empty() ?
                            string("Sale failed. Check product, warehouse, and component stock.") :
                            "Sale failed. " + inventoryManager.getLastError());
                    }


                    autoSaveIfEnabled(warehouseSystem);


                    // ------------------------------------------------
                    // Response
                    // ------------------------------------------------

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Product sold successfully.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// PROCUREMENT - LIST
// ============================================================

    CROW_ROUTE(app, "/api/procurement")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list orderList;


                ProcurementManager& procurementManager =
                    warehouseSystem->getProcurementManager();

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();


                for (const auto& order :
                    procurementManager.getOrders())
                {
                    crow::json::wvalue item;

                    item["id"] =
                        order->getID();

                    item["productID"] =
                        order->getProductID();

                    item["materialID"] =
                        order->getMaterialID();


                    Material* material =
                        materialManager.findMaterial(
                            order->getMaterialID());

                    // Prefer the live catalog name (picks up a
                    // rename), but fall back to the name snapshot
                    // saved on the order itself if the Material was
                    // since deleted from the catalog.
                    item["materialName"] =
                        (material != nullptr) ?
                        material->getName() :
                        order->getMaterialName();


                    Supplier* supplier =
                        (material != nullptr) ?
                        material->getSupplier() : nullptr;

                    item["supplierName"] =
                        (supplier != nullptr) ?
                        supplier->getName() : "No Supplier";

                    // The primary Supplier's own part number for this
                    // material - what the Supplier knows it by, so it
                    // is shown on the order lines
                    item["supplierPartNumber"] =
                        (material != nullptr) ?
                        material->getSupplierPartNumber() : "";


                    item["warehouseID"] =
                        order->getWarehouseID();

                    item["orderDate"] =
                        order->getOrderDate();

                    item["orderedQuantity"] =
                        order->getOrderedQuantity();

                    item["comment"] =
                        order->getComment();

                    item["confirmationDate"] =
                        order->getConfirmationDate();

                    item["confirmedQuantity"] =
                        order->getConfirmedQuantity();

                    item["estimatedReceiptDate"] =
                        order->getEstimatedReceiptDate();

                    item["totalReceivedQuantity"] =
                        order->getTotalReceivedQuantity();

                    item["pendingQuantity"] =
                        order->getPendingQuantity();

                    item["status"] =
                        order->getStatus();

                    // Shared per "PRC-######", not per line - see
                    // ProcurementManager::setOrderConfirmationPath().

                    item["orderConfirmationPath"] =
                        procurementManager.getOrderConfirmationPath(
                            order->getID());

                    crow::json::wvalue::list lieferscheinList;

                    for (const string& path :
                        procurementManager.getLieferscheinPaths(
                            order->getID()))
                    {
                        lieferscheinList.push_back(path);
                    }

                    item["lieferscheinPaths"] =
                        std::move(lieferscheinList);


                    crow::json::wvalue::list receiptList;

                    for (const auto& receipt :
                        order->getReceipts())
                    {
                        crow::json::wvalue r;

                        r["receiptDate"] =
                            receipt.receiptDate;

                        r["receivedQuantity"] =
                            receipt.receivedQuantity;

                        r["comment"] =
                            receipt.comment;

                        receiptList.push_back(
                            std::move(r));
                    }

                    item["receipts"] =
                        std::move(receiptList);


                    orderList.push_back(
                        std::move(item));
                }


                response["orders"] =
                    std::move(orderList);


                return response;
            });

// ============================================================
// PROCUREMENT DOCUMENTS - UPLOAD ORDER CONFIRMATION
// ============================================================
// One Order Confirmation PDF per whole "PRC-######" (shared across
// every material line) - always allowed once the order exists, since
// a confirmation from the supplier is expected right after placing
// it. Uploading again replaces the previous file.

    CROW_ROUTE(app, "/api/procurement/<string>/documents/order-confirmation")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req, string id)
            {
                try
                {
                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();

                    if (procurementManager.findOrder(id) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Procurement Order not found.");
                    }

                    string documentsFolder =
                        warehouseSystem->getDataManager()
                            .getDocumentsFolder();

                    if (documentsFolder.empty())
                    {
                        return crow::response(
                            400,
                            "Set a Documents Folder first in Data "
                            "Management Settings.");
                    }

                    auto body =
                        crow::json::load(req.body);

                    if (!body || !body.has("fileData"))
                    {
                        return crow::response(
                            400,
                            "fileData is required.");
                    }

                    string fileData =
                        body["fileData"].s();

                    size_t commaPosition =
                        fileData.find(',');

                    if (commaPosition == string::npos)
                    {
                        return crow::response(
                            400,
                            "Invalid file data.");
                    }

                    string decoded =
                        decodeBase64(
                            fileData.substr(commaPosition + 1));

                    // Saved directly under the Documents Folder root,
                    // named after the PRC number itself (e.g.
                    // "PRC-000001 - confirmation.pdf") rather than in
                    // a per-order subfolder - easier to spot when
                    // browsing that folder directly.

                    fs::create_directories(documentsFolder);

                    fs::path filePath =
                        fs::path(documentsFolder) /
                        (id + " - confirmation.pdf");

                    ofstream outFile(
                        filePath,
                        ios::binary);

                    if (!outFile)
                    {
                        return crow::response(
                            500,
                            "Could not write file.");
                    }

                    outFile.write(
                        decoded.data(),
                        decoded.size());

                    outFile.close();

                    procurementManager.setOrderConfirmationPath(
                        id,
                        filePath.string());

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["path"] =
                        filePath.string();

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// PROCUREMENT DOCUMENTS - UPLOAD LIEFERSCHEIN
// ============================================================
// Any number of Lieferschein (delivery note) PDFs per whole
// "PRC-######" - one per shipment, since a partial delivery can
// arrive in several batches - allowed only once the order has been
// confirmed by the supplier (at least one line has a Confirmation
// Date) - just for keeping the paperwork lined up, not a real
// receiving step. Each upload always adds another file, never
// replaces a previous one.

    CROW_ROUTE(app, "/api/procurement/<string>/documents/lieferschein")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req, string id)
            {
                try
                {
                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();

                    vector<ProcurementOrder*> lines =
                        procurementManager.findOrderLines(id);

                    if (lines.empty())
                    {
                        return crow::response(
                            404,
                            "Procurement Order not found.");
                    }

                    bool anyConfirmed =
                        any_of(
                            lines.begin(),
                            lines.end(),
                            [](ProcurementOrder* line)
                            {
                                return !line->getConfirmationDate().empty();
                            });

                    if (!anyConfirmed)
                    {
                        return crow::response(
                            400,
                            "Confirm the order first before attaching a "
                            "Lieferschein.");
                    }

                    string documentsFolder =
                        warehouseSystem->getDataManager()
                            .getDocumentsFolder();

                    if (documentsFolder.empty())
                    {
                        return crow::response(
                            400,
                            "Set a Documents Folder first in Data "
                            "Management Settings.");
                    }

                    auto body =
                        crow::json::load(req.body);

                    if (!body || !body.has("fileData"))
                    {
                        return crow::response(
                            400,
                            "fileData is required.");
                    }

                    string fileData =
                        body["fileData"].s();

                    size_t commaPosition =
                        fileData.find(',');

                    if (commaPosition == string::npos)
                    {
                        return crow::response(
                            400,
                            "Invalid file data.");
                    }

                    string decoded =
                        decodeBase64(
                            fileData.substr(commaPosition + 1));

                    // Same flat layout as Order Confirmation - saved
                    // directly under the Documents Folder root, named
                    // "PRC-000001 - DeliveryNote1.pdf",
                    // "PRC-000001 - DeliveryNote2.pdf", etc. (the
                    // next sequential number after however many are
                    // already attached), so they sit right next to
                    // that order's confirmation file, one after
                    // another, when browsing the folder.

                    fs::create_directories(documentsFolder);

                    int nextIndex =
                        (int)procurementManager
                            .getLieferscheinPaths(id).size() + 1;

                    fs::path filePath =
                        fs::path(documentsFolder) /
                        (id + " - DeliveryNote" +
                            to_string(nextIndex) + ".pdf");

                    ofstream outFile(
                        filePath,
                        ios::binary);

                    if (!outFile)
                    {
                        return crow::response(
                            500,
                            "Could not write file.");
                    }

                    outFile.write(
                        decoded.data(),
                        decoded.size());

                    outFile.close();

                    procurementManager.addLieferscheinPath(
                        id,
                        filePath.string());

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["path"] =
                        filePath.string();

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// PROCUREMENT DOCUMENTS - VIEW (serve the PDF bytes)
// ============================================================

    CROW_ROUTE(app, "/api/procurement/<string>/documents/order-confirmation/file")
        ([warehouseSystem](string id)
            {
                string path =
                    warehouseSystem->getProcurementManager()
                        .getOrderConfirmationPath(id);

                if (path.empty() || !fs::exists(path))
                {
                    return crow::response(
                        404,
                        "No Order Confirmation on file for this order.");
                }

                ifstream inFile(
                    path,
                    ios::binary);

                ostringstream contents;

                contents << inFile.rdbuf();

                crow::response res(contents.str());

                res.set_header(
                    "Content-Type",
                    "application/pdf");

                return res;
            });

    CROW_ROUTE(app, "/api/procurement/<string>/documents/lieferschein/<int>/file")
        ([warehouseSystem](string id, int index)
            {
                vector<string> paths =
                    warehouseSystem->getProcurementManager()
                        .getLieferscheinPaths(id);

                // index is 1-based, matching the DeliveryNote1/2/3...
                // numbering used in the saved filenames.

                if (index < 1 ||
                    (size_t)index > paths.size())
                {
                    return crow::response(
                        404,
                        "No Lieferschein with that number on file for "
                        "this order.");
                }

                string path = paths[index - 1];

                if (path.empty() || !fs::exists(path))
                {
                    return crow::response(
                        404,
                        "No Lieferschein on file for this order.");
                }

                ifstream inFile(
                    path,
                    ios::binary);

                ostringstream contents;

                contents << inFile.rdbuf();

                crow::response res(contents.str());

                res.set_header(
                    "Content-Type",
                    "application/pdf");

                return res;
            });

// ============================================================
// PROJECTIONS - LIST
// ============================================================

    CROW_ROUTE(app, "/api/projections")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list projectionList;


                ProjectionManager& projectionManager =
                    warehouseSystem->getProjectionManager();

                ProductManager& productManager =
                    warehouseSystem->getProductManager();


                for (const auto& projection :
                    projectionManager.getProjections())
                {
                    // Pull in any material that has newly become
                    // short since this Projection was created or last
                    // refreshed (see ProjectionManager::
                    // refreshShortfallItems()), so a still-open
                    // Projection never permanently "misses" a
                    // shortfall just because it wasn't one yet back
                    // when the Projection was created.

                    if (!projection->isCompleted())
                    {
                        projectionManager.refreshShortfallItems(
                            projection->getID());
                    }

                    crow::json::wvalue item;

                    item["id"] =
                        projection->getID();

                    item["productID"] =
                        projection->getProductID();


                    Product* product =
                        productManager.findProduct(
                            projection->getProductID());

                    item["productName"] =
                        (product != nullptr) ?
                        product->getName() : "";


                    item["warehouseID"] =
                        projection->getWarehouseID();

                    item["deadline"] =
                        projection->getDeadline();

                    item["manufactureQuantity"] =
                        projection->getManufactureQuantity();

                    item["creationDate"] =
                        projection->getCreationDate();


                    // Status: "Fully Ordered" once every item's "Qty
                    // to Order", AS SEEN BY THIS Projection (itself
                    // plus every open Projection due at or before its
                    // own deadline - see ProjectionManager::
                    // getQuantityToOrder() for the reasoning), has
                    // reached zero.

                    bool fullyOrdered = true;

                    for (const auto& projItem :
                        projection->getItems())
                    {
                        double toOrder =
                            projectionManager.getQuantityToOrder(
                                projItem.materialID,
                                projection->getWarehouseID(),
                                projection->getDeadline());

                        if (toOrder > 0 &&
                            !projItem.orderRegistered)
                        {
                            fullyOrdered = false;
                            break;
                        }
                    }

                    item["status"] =
                        fullyOrdered ?
                        "Fully Ordered" : "Pending Orders";

                    item["completed"] =
                        projection->isCompleted();

                    item["cancelled"] =
                        projection->isCancelled();

                    item["producedQuantity"] =
                        projection->getProducedQuantity();

                    item["completionDate"] =
                        projection->getCompletionDate();


                    projectionList.push_back(
                        std::move(item));
                }


                response["projections"] =
                    std::move(projectionList);


                return response;
            });

// ============================================================
// PROJECTIONS - DETAIL
// ============================================================

    CROW_ROUTE(app, "/api/projections/<string>")
        ([warehouseSystem](const string& id)
            {
                ProjectionManager& projectionManager =
                    warehouseSystem->getProjectionManager();

                Projection* projection =
                    projectionManager.findProjection(id);

                if (projection == nullptr)
                {
                    return crow::response(
                        404,
                        "Projection not found.");
                }


                // Same live "pick up new shortfalls" refresh as the
                // list route above - see ProjectionManager::
                // refreshShortfallItems().

                if (!projection->isCompleted())
                {
                    projectionManager.refreshShortfallItems(id);
                }


                ProcurementManager& procurementManager =
                    warehouseSystem->getProcurementManager();

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();

                ProductManager& productManager =
                    warehouseSystem->getProductManager();


                crow::json::wvalue response;

                response["id"] =
                    projection->getID();

                response["productID"] =
                    projection->getProductID();


                Product* product =
                    productManager.findProduct(
                        projection->getProductID());

                response["productName"] =
                    (product != nullptr) ?
                    product->getName() : "";


                response["warehouseID"] =
                    projection->getWarehouseID();

                response["deadline"] =
                    projection->getDeadline();

                response["manufactureQuantity"] =
                    projection->getManufactureQuantity();

                response["creationDate"] =
                    projection->getCreationDate();


                crow::json::wvalue::list itemList;

                bool fullyOrdered = true;

                for (const auto& projItem :
                    projection->getItems())
                {
                    // "Qty to Order": the live total needed for this
                    // material, AS SEEN BY THIS Projection - itself
                    // plus every other currently open Projection tied
                    // to this same Warehouse whose OWN deadline is AT
                    // OR BEFORE this Projection's deadline, never one
                    // due later (see ProjectionManager::
                    // getQuantityToOrder()) - minus that Warehouse's
                    // actual stock right now. This avoids the earlier
                    // double-counting (each Projection independently
                    // believing it alone needed to cover the whole
                    // shortfall) while also never letting a Projection
                    // due later change one due sooner's own numbers.

                    double pending =
                        projectionManager.getQuantityToOrder(
                            projItem.materialID,
                            projection->getWarehouseID(),
                            projection->getDeadline());

                    // Once this line has been frozen as "already
                    // ordered" (see ProjectionItem::orderRegistered),
                    // it counts as done for status purposes even if
                    // the live "Qty to Order" above is still above
                    // zero (e.g. another Projection due at or before it
                    // is now also competing for the same material) -
                    // the freeze is permanent until this Projection is
                    // completed.

                    if (pending > 0 &&
                        !projItem.orderRegistered)
                    {
                        fullyOrdered = false;
                    }

                    // Outstanding Ordered Quantity: purely
                    // informational, shown alongside "Qty to Order"
                    // but never subtracted from it - see
                    // getOutstandingOrderedQuantity() above.

                    double ordered =
                        getOutstandingOrderedQuantity(
                            projItem.materialID,
                            projection->getWarehouseID(),
                            procurementManager);

                    // Actual current stock of this material in this
                    // Projection's Warehouse (not netted against other
                    // Projections - that is what "Qty to Order" above
                    // already accounts for).

                    double stockForPending =
                        projectionManager.getWarehouseStock(
                            projItem.materialID,
                            projection->getWarehouseID());


                    crow::json::wvalue itemJson;

                    itemJson["materialID"] =
                        projItem.materialID;


                    Material* material =
                        materialManager.findMaterial(
                            projItem.materialID);

                    itemJson["materialName"] =
                        (material != nullptr) ?
                        material->getName() : "";

                    itemJson["uom"] =
                        (material != nullptr) ?
                        material->getUoM() : "";

                    itemJson["photo"] =
                        (material != nullptr) ?
                        material->getPhotoPath() : "";


                    // Supplier name - used by the frontend to group
                    // this Projection's materials by Supplier. Empty
                    // when the Material has none assigned yet.

                    Supplier* itemSupplier =
                        (material != nullptr) ?
                        material->getSupplier() : nullptr;

                    itemJson["supplierName"] =
                        (itemSupplier != nullptr) ?
                        itemSupplier->getName() : "";


                    itemJson["requiredQuantity"] =
                        projItem.requiredQuantity;

                    itemJson["stockAtCreation"] =
                        projItem.stockAtCreation;


                    // Actual current stock (live) of this material in
                    // this Projection's Warehouse - see comment above
                    // computing stockForPending.

                    itemJson["currentStock"] =
                        stockForPending;


                    // Informational only, never netted against
                    // "pendingQuantity" (Qty to Order) - see
                    // getOutstandingOrderedQuantity() above.

                    itemJson["orderedQuantity"] =
                        ordered;

                    itemJson["pendingQuantity"] =
                        pending;

                    // Frozen "already ordered" state - see
                    // ProjectionItem::orderRegistered. The frontend
                    // uses this (not a live comparison) to decide
                    // whether this row can still be selected to
                    // register another order.
                    itemJson["orderRegistered"] =
                        projItem.orderRegistered;


                    // Order-by timing: deadline minus this
                    // material's current supplier's lead time.

                    string timingStatus;
                    string orderByDate;

                    computeOrderTiming(
                        projection->getDeadline(),
                        (material != nullptr) ?
                            material->getSupplier() : nullptr,
                        timingStatus,
                        orderByDate);

                    itemJson["orderByDate"] =
                        orderByDate;

                    itemJson["timingStatus"] =
                        timingStatus;


                    itemList.push_back(
                        std::move(itemJson));
                }

                response["items"] =
                    std::move(itemList);

                response["status"] =
                    fullyOrdered ?
                    "Fully Ordered" : "Pending Orders";

                response["completed"] =
                    projection->isCompleted();

                response["cancelled"] =
                    projection->isCancelled();

                response["producedQuantity"] =
                    projection->getProducedQuantity();

                response["completionDate"] =
                    projection->getCompletionDate();


                return crow::response(response);
            });

// ============================================================
// PROJECTIONS - COMPLETE (confirm production, issue BOM)
// ============================================================

    CROW_ROUTE(app, "/api/projections/complete")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string id =
                        body["id"].s();

                    int producedQuantity =
                        body["producedQuantity"].i();

                    string completionDate =
                        body["completionDate"].s();

                    if (id.empty())
                    {
                        return crow::response(
                            400,
                            "Projection ID is required.");
                    }

                    if (completionDate.empty())
                    {
                        return crow::response(
                            400,
                            "Completion Date is required.");
                    }

                    if (producedQuantity <= 0)
                    {
                        return crow::response(
                            400,
                            "Produced Quantity must be greater than zero.");
                    }

                    ProjectionManager& projectionManager =
                        warehouseSystem->getProjectionManager();

                    Projection* projection =
                        projectionManager.findProjection(id);

                    if (projection == nullptr)
                    {
                        return crow::response(
                            404,
                            "Projection not found.");
                    }

                    if (projection->isCompleted())
                    {
                        return crow::response(
                            409,
                            "This Projection has already been completed.");
                    }

                    {
                        string lockedError =
                            warehouseLockedError(
                                warehouseSystem,
                                projection->getWarehouseID());

                        if (!lockedError.empty())
                        {
                            return crow::response(
                                409,
                                lockedError);
                        }
                    }

                    bool success =
                        projectionManager.completeProjection(
                            id,
                            producedQuantity,
                            completionDate);

                    if (!success)
                    {
                        return crow::response(
                            400,
                            projectionManager.getLastCompleteError().empty() ?
                            string("Could not confirm production. Check that every BOM material has enough stock for the produced quantity.") :
                            "Could not confirm production. " + projectionManager.getLastCompleteError());
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Production confirmed and materials issued.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// PROJECTIONS - DELETE
// ============================================================
// Scraps a Projection plan outright. Procurement Orders already
// placed from it are left alone (see ProjectionManager::
// deleteProjection) - only the still-open plan disappears.

    CROW_ROUTE(app, "/api/projections/delete")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string id =
                        body["id"].s();

                    if (id.empty())
                    {
                        return crow::response(
                            400,
                            "Projection ID is required.");
                    }

                    ProjectionManager& projectionManager =
                        warehouseSystem->getProjectionManager();

                    Projection* projection =
                        projectionManager.findProjection(id);

                    if (projection == nullptr)
                    {
                        return crow::response(
                            404,
                            "Projection not found.");
                    }

                    if (projection->isCompleted())
                    {
                        return crow::response(
                            409,
                            "This Projection is already archived (completed or cancelled).");
                    }

                    bool success =
                        projectionManager.deleteProjection(id);

                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Could not cancel this Projection.");
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Projection cancelled and moved to Archived Projections.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// PROJECTIONS - CREATE
// ============================================================

    CROW_ROUTE(app, "/api/projections/create")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    string productID =
                        body["productID"].s();

                    // The Warehouse is no longer supplied by the
                    // caller - Projection creation always uses the
                    // Product's own Main Warehouse (see
                    // ProjectionManager::createProjection()).

                    string deadline = "";

                    if (body.has("deadline"))
                    {
                        deadline =
                            body["deadline"].s();
                    }

                    int manufactureQuantity =
                        body["manufactureQuantity"].i();


                    // ------------------------------------------------
                    // Validate
                    // ------------------------------------------------

                    ProductManager& productManager =
                        warehouseSystem->getProductManager();

                    if (productManager.findProduct(productID) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Product not found.");
                    }


                    if (manufactureQuantity <= 0)
                    {
                        return crow::response(
                            400,
                            "Quantity to Manufacture must be greater than zero.");
                    }


                    // ------------------------------------------------
                    // Create projection
                    // ------------------------------------------------

                    time_t now = time(nullptr);
                    char dateBuffer[11];

                    strftime(
                        dateBuffer,
                        sizeof(dateBuffer),
                        "%Y-%m-%d",
                        localtime(&now));


                    ProjectionManager& projectionManager =
                        warehouseSystem->getProjectionManager();

                    string errorMessage;

                    Projection* projection =
                        projectionManager.createProjection(
                            productID,
                            deadline,
                            manufactureQuantity,
                            string(dateBuffer),
                            errorMessage);


                    if (projection == nullptr)
                    {
                        return crow::response(
                            400,
                            errorMessage);
                    }


                    // ------------------------------------------------
                    // Response
                    // ------------------------------------------------

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["id"] =
                        projection->getID();

                    response["message"] =
                        "Projection created successfully.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// PROCUREMENT - CREATE ORDER
// ============================================================

    CROW_ROUTE(app, "/api/procurement/create")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    // ------------------------------------------------
                    // Read data
                    // ------------------------------------------------

                    string productID = "";

                    if (body.has("productID"))
                    {
                        productID =
                            body["productID"].s();
                    }

                    string materialID =
                        body["materialID"].s();

                    int warehouseID =
                        body["warehouseID"].i();

                    string orderDate =
                        body["orderDate"].s();

                    double orderedQuantity =
                        jsonQuantity(body["orderedQuantity"]);

                    {
                        string decimalsError =
                            quantityDecimalsError(orderedQuantity, "Ordered Quantity");

                        if (!decimalsError.empty())
                        {
                            return crow::response(
                                400,
                                decimalsError);
                        }
                    }

                    string comment = "";

                    if (body.has("comment"))
                    {
                        comment =
                            body["comment"].s();
                    }

                    // Internal only - which Projection batch (if
                    // any) this order comes from. Not required.
                    string projectionID = "";

                    if (body.has("projectionID"))
                    {
                        projectionID =
                            body["projectionID"].s();
                    }


                    // ------------------------------------------------
                    // Validate
                    // ------------------------------------------------

                    if (!Material::isValidID(materialID))
                    {
                        return crow::response(
                            400,
                            "Invalid Material ID. Expected format ###-###### or ######-00.");
                    }


                    MaterialManager& materialManager =
                        warehouseSystem->getMaterialManager();


                    Material* material =
                        materialManager.findMaterial(materialID);

                    if (material == nullptr)
                    {
                        return crow::response(
                            404,
                            "Material not found.");
                    }


                    // A Material can be registered without a
                    // Supplier (e.g. still being sourced), but a
                    // Procurement Order is a real purchase - it
                    // needs a Supplier to buy from.

                    if (material->getSupplier() == nullptr)
                    {
                        return crow::response(
                            400,
                            "This Material has no Supplier assigned yet. "
                            "Please assign a Supplier in Modify Material "
                            "before creating a Procurement Order for it.");
                    }

                    // A Material made by an Internal Supplier (e.g.
                    // "4Tex GmbH") is internal work, not a real
                    // purchase - it cannot go through a Procurement
                    // Order at all.

                    if (material->hasInternalSupplier())
                    {
                        return crow::response(
                            400,
                            "This Material is made by an Internal Supplier ("
                            + material->getSupplier()->getName() +
                            "). It is internal work, not a purchase, so it "
                            "cannot be ordered through a Procurement Order.");
                    }


                    WarehouseManager& warehouseManager =
                        warehouseSystem->getWarehouseManager();


                    if (warehouseManager.findWarehouse(warehouseID) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Warehouse not found.");
                    }


                    if (orderDate.empty())
                    {
                        return crow::response(
                            400,
                            "Order Date is required.");
                    }


                    if (orderedQuantity <= 0)
                    {
                        return crow::response(
                            400,
                            "Ordered Quantity must be greater than zero.");
                    }


                    // ------------------------------------------------
                    // Create order
                    // ------------------------------------------------

                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();


                    ProcurementOrder* order =
                        procurementManager.createOrder(
                            productID,
                            materialID,
                            warehouseID,
                            orderDate,
                            orderedQuantity,
                            comment,
                            projectionID);


                    if (order == nullptr)
                    {
                        return crow::response(
                            400,
                            "Could not create Procurement Order.");
                    }


                    // ------------------------------------------------
                    // Response
                    // ------------------------------------------------

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["id"] =
                        order->getID();

                    response["message"] =
                        "Procurement Order created successfully.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// PROCUREMENT - CREATE BATCH (several materials, one shared
// "PRC-######" number - one action = one Procurement Order)
// ============================================================
// Body: { productID, warehouseID, orderDate, comment, projectionID,
//         lines: [ { materialID, orderedQuantity }, ... ] }
// All lines are created under one reserved ID. If any line fails
// validation the whole batch is rejected before anything is
// created, so a bad line can't leave a half-created order behind.

    CROW_ROUTE(app, "/api/procurement/create-batch")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    string productID = "";

                    if (body.has("productID"))
                    {
                        productID =
                            body["productID"].s();
                    }

                    int warehouseID =
                        body["warehouseID"].i();

                    string orderDate =
                        body["orderDate"].s();

                    string comment = "";

                    if (body.has("comment"))
                    {
                        comment =
                            body["comment"].s();
                    }

                    string projectionID = "";

                    if (body.has("projectionID"))
                    {
                        projectionID =
                            body["projectionID"].s();
                    }


                    if (orderDate.empty())
                    {
                        return crow::response(
                            400,
                            "Order Date is required.");
                    }


                    if (!body.has("lines"))
                    {
                        return crow::response(
                            400,
                            "At least one material line is required.");
                    }


                    WarehouseManager& warehouseManager =
                        warehouseSystem->getWarehouseManager();

                    if (warehouseManager.findWarehouse(warehouseID) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Warehouse not found.");
                    }


                    MaterialManager& materialManager =
                        warehouseSystem->getMaterialManager();


                    // ------------------------------------------------
                    // Validate every line before creating anything
                    // ------------------------------------------------

                    struct BatchLine
                    {
                        string materialID;
                        double orderedQuantity;
                        string supplierName;
                    };

                    vector<BatchLine> lines;

                    for (const auto& lineJson : body["lines"])
                    {
                        string materialID =
                            lineJson["materialID"].s();

                        double orderedQuantity =
                            jsonQuantity(lineJson["orderedQuantity"]);

                        {
                            string decimalsError =
                                quantityDecimalsError(orderedQuantity, "Ordered Quantity");

                            if (!decimalsError.empty())
                            {
                                return crow::response(
                                    400,
                                    decimalsError);
                            }
                        }

                        if (!Material::isValidID(materialID))
                        {
                            return crow::response(
                                400,
                                "Invalid Material ID: " + materialID);
                        }

                        Material* lineMaterial =
                            materialManager.findMaterial(materialID);

                        if (lineMaterial == nullptr)
                        {
                            return crow::response(
                                404,
                                "Material not found: " + materialID);
                        }

                        if (lineMaterial->getSupplier() == nullptr)
                        {
                            return crow::response(
                                400,
                                "Material " + materialID
                                + " has no Supplier assigned yet. Please "
                                "assign a Supplier in Modify Material "
                                "before creating a Procurement Order for it.");
                        }

                        if (lineMaterial->hasInternalSupplier())
                        {
                            return crow::response(
                                400,
                                "Material " + materialID
                                + " is made by an Internal Supplier ("
                                + lineMaterial->getSupplier()->getName() +
                                "). It is internal work, not a purchase, so "
                                "it cannot be ordered through a Procurement "
                                "Order.");
                        }

                        if (orderedQuantity <= 0)
                        {
                            return crow::response(
                                400,
                                "Ordered Quantity must be greater than "
                                "zero for material " + materialID);
                        }

                        lines.push_back({
                            materialID,
                            orderedQuantity,
                            lineMaterial->getSupplier()->getName()});
                    }


                    if (lines.empty())
                    {
                        return crow::response(
                            400,
                            "At least one material line is required.");
                    }


                    // ------------------------------------------------
                    // Create - all lines share one reserved ID
                    // ------------------------------------------------

                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();

                    // One Procurement Order belongs to exactly one
                    // Supplier (the Supplier heading, Order
                    // Confirmation and Lieferschein are all per
                    // Supplier - see also the add-material route). So
                    // lines of different Suppliers are split into one
                    // "PRC-######" per Supplier, in the order the
                    // Suppliers first appear. Each ID is reserved only
                    // after the previous group was created, because
                    // reserveNextID() derives it from existing orders.

                    vector<string> supplierNames;

                    for (const auto& line : lines)
                    {
                        if (find(supplierNames.begin(),
                            supplierNames.end(),
                            line.supplierName) == supplierNames.end())
                        {
                            supplierNames.push_back(line.supplierName);
                        }
                    }

                    vector<string> createdIDs;

                    for (const auto& supplierName : supplierNames)
                    {
                        string batchID =
                            procurementManager.reserveNextID();

                        for (const auto& line : lines)
                        {
                            if (line.supplierName != supplierName)
                            {
                                continue;
                            }

                            procurementManager.createOrder(
                                productID,
                                line.materialID,
                                warehouseID,
                                orderDate,
                                line.orderedQuantity,
                                comment,
                                projectionID,
                                batchID);
                        }

                        createdIDs.push_back(batchID);
                    }


                    // If this batch was registered from a Projection,
                    // record one order-placement attempt against each
                    // line ordered - see Projection::
                    // registerOrderAttempt(). A line only ever gets at
                    // most two such attempts from the Projection view:
                    // it freezes immediately if outstanding orders now
                    // cover the shared "Qty to Order" shortfall in
                    // full, or otherwise once this was its second
                    // attempt regardless of whether it was enough -
                    // anything still missing after that is placed
                    // manually from New Order. Once frozen, a line
                    // stays closed for ordering from this Projection
                    // even if stock or orders shift again later (e.g.
                    // an order gets cancelled), until the Projection
                    // is completed.

                    if (!projectionID.empty())
                    {
                        ProjectionManager& projectionManager =
                            warehouseSystem->getProjectionManager();

                        Projection* sourceProjection =
                            projectionManager.findProjection(
                                projectionID);

                        if (sourceProjection != nullptr)
                        {
                            for (const auto& line : lines)
                            {
                                bool isProjectionItem = false;

                                for (const auto& projItem :
                                    sourceProjection->getItems())
                                {
                                    if (projItem.materialID ==
                                        line.materialID)
                                    {
                                        isProjectionItem = true;
                                        break;
                                    }
                                }

                                if (!isProjectionItem)
                                {
                                    continue;
                                }

                                // "Fully covered" has to compare
                                // against the "Qty to Order" shortfall
                                // AS SEEN BY THIS SOURCE PROJECTION
                                // (stock already subtracted, itself
                                // plus every open Projection due at or
                                // before its own deadline - see
                                // ProjectionManager::
                                // getQuantityToOrder()), NOT against
                                // this line's own raw requiredQuantity
                                // - requiredQuantity ignores stock
                                // entirely, so a line that only ever
                                // needed the shortfall amount (e.g.
                                // Required 30, Stock 20, Qty to Order
                                // 10) would never read as covered even
                                // after ordering exactly the 10 it
                                // actually needed.

                                double neededQuantity =
                                    projectionManager.getQuantityToOrder(
                                        line.materialID,
                                        warehouseID,
                                        sourceProjection->getDeadline());

                                double outstandingOrdered =
                                    getOutstandingOrderedQuantity(
                                        line.materialID,
                                        warehouseID,
                                        procurementManager);

                                bool fullyCovered =
                                    !qtyLess(
                                        outstandingOrdered,
                                        neededQuantity);

                                projectionManager
                                    .registerItemOrderAttempt(
                                        projectionID,
                                        line.materialID,
                                        fullyCovered);
                            }
                        }
                    }


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["id"] = createdIDs.front();

                    string createdList;

                    for (size_t i = 0; i < createdIDs.size(); i++)
                    {
                        if (i > 0)
                        {
                            createdList += ", ";
                        }

                        createdList += createdIDs[i];
                    }

                    response["ids"] = createdList;

                    response["message"] =
                        createdIDs.size() > 1 ?
                        "Procurement Orders created (one per Supplier): "
                        + createdList + "." :
                        "Procurement Order created successfully.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// PROCUREMENT - ADD MATERIAL TO AN EXISTING (UNCONFIRMED) ORDER
// ============================================================
// Sometimes another Material for the same Supplier is only needed
// after the "PRC-######" was already placed - rather than forcing a
// brand new Order (and a new Supplier confirmation/Lieferschein
// cycle) for it, this adds one more line straight onto the existing
// Order, sharing its Warehouse and Order Date. Only allowed while
// the Order is still fully unconfirmed (Open Orders) - once the
// Supplier has confirmed anything on it, appending a line would be
// changing an order they already agreed to.
// Body: { materialID, orderedQuantity, comment }

    CROW_ROUTE(app, "/api/procurement/<string>/add-material")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req, string id)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    string materialID =
                        body["materialID"].s();

                    double orderedQuantity =
                        jsonQuantity(body["orderedQuantity"]);

                    {
                        string decimalsError =
                            quantityDecimalsError(orderedQuantity, "Ordered Quantity");

                        if (!decimalsError.empty())
                        {
                            return crow::response(
                                400,
                                decimalsError);
                        }
                    }

                    string comment = "";

                    if (body.has("comment"))
                    {
                        comment =
                            body["comment"].s();
                    }


                    // ------------------------------------------------
                    // Existing order must already exist and still be
                    // fully unconfirmed
                    // ------------------------------------------------

                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();

                    vector<ProcurementOrder*> existingLines =
                        procurementManager.findOrderLines(id);

                    if (existingLines.empty())
                    {
                        return crow::response(
                            404,
                            "Procurement Order not found.");
                    }

                    for (ProcurementOrder* line : existingLines)
                    {
                        if (line->getStatus() != "Ordered")
                        {
                            return crow::response(
                                400,
                                "Materials can only be added to an Order "
                                "that has not been confirmed yet.");
                        }

                        if (line->getMaterialID() == materialID)
                        {
                            return crow::response(
                                400,
                                "This Material is already part of this "
                                "Procurement Order. Use its Qty field to "
                                "change the quantity instead.");
                        }
                    }


                    // ------------------------------------------------
                    // Validate the new Material the same way as when
                    // creating an Order
                    // ------------------------------------------------

                    if (!Material::isValidID(materialID))
                    {
                        return crow::response(
                            400,
                            "Invalid Material ID. Expected format ###-###### or ######-00.");
                    }

                    MaterialManager& materialManager =
                        warehouseSystem->getMaterialManager();

                    Material* material =
                        materialManager.findMaterial(materialID);

                    if (material == nullptr)
                    {
                        return crow::response(
                            404,
                            "Material not found.");
                    }

                    if (material->getSupplier() == nullptr)
                    {
                        return crow::response(
                            400,
                            "This Material has no Supplier assigned yet. "
                            "Please assign a Supplier in Modify Material "
                            "before adding it to a Procurement Order.");
                    }

                    if (material->hasInternalSupplier())
                    {
                        return crow::response(
                            400,
                            "This Material is made by an Internal Supplier ("
                            + material->getSupplier()->getName() +
                            "). It is internal work, not a purchase, so it "
                            "cannot be ordered through a Procurement Order.");
                    }


                    // Every line of one "PRC-######" is shown under one
                    // Supplier heading in the UI - a new line for a
                    // different Supplier would be misleading there, and
                    // an Order Confirmation/Lieferschein from Supplier A
                    // should not end up covering a line meant for
                    // Supplier B.

                    Material* firstLineMaterial =
                        materialManager.findMaterial(
                            existingLines[0]->getMaterialID());

                    Supplier* orderSupplier =
                        (firstLineMaterial != nullptr) ?
                        firstLineMaterial->getSupplier() : nullptr;

                    string orderSupplierName =
                        (orderSupplier != nullptr) ?
                        orderSupplier->getName() : "";

                    string newMaterialSupplierName =
                        material->getSupplier()->getName();

                    if (!orderSupplierName.empty() &&
                        newMaterialSupplierName != orderSupplierName)
                    {
                        return crow::response(
                            400,
                            "This Material's Supplier (" +
                            newMaterialSupplierName +
                            ") does not match this Order's Supplier (" +
                            orderSupplierName +
                            "). Create a separate Procurement Order for a "
                            "different Supplier.");
                    }


                    if (orderedQuantity <= 0)
                    {
                        return crow::response(
                            400,
                            "Ordered Quantity must be greater than zero.");
                    }


                    // ------------------------------------------------
                    // Add the line - same Warehouse and Order Date as
                    // the rest of this Order, not tied to any Product/
                    // Projection since it was added independently.
                    // ------------------------------------------------

                    ProcurementOrder* newLine =
                        procurementManager.createOrder(
                            "",
                            materialID,
                            existingLines[0]->getWarehouseID(),
                            existingLines[0]->getOrderDate(),
                            orderedQuantity,
                            comment,
                            "",
                            id);

                    if (newLine == nullptr)
                    {
                        return crow::response(
                            400,
                            "Could not add Material to this Procurement Order.");
                    }


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["id"] = id;

                    response["message"] =
                        "Material added to the Procurement Order.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// PROCUREMENT - CONFIRM ORDER
// ============================================================

    CROW_ROUTE(app, "/api/procurement/confirm")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    string id =
                        body["id"].s();

                    string materialID =
                        body["materialID"].s();

                    string confirmationDate =
                        body["confirmationDate"].s();

                    double confirmedQuantity =
                        jsonQuantity(body["confirmedQuantity"]);

                    {
                        string decimalsError =
                            quantityDecimalsError(confirmedQuantity, "Confirmed Quantity");

                        if (!decimalsError.empty())
                        {
                            return crow::response(
                                400,
                                decimalsError);
                        }
                    }


                    if (id.empty())
                    {
                        return crow::response(
                            400,
                            "Procurement Order ID is required.");
                    }


                    if (materialID.empty())
                    {
                        return crow::response(
                            400,
                            "Material ID is required.");
                    }


                    if (confirmationDate.empty())
                    {
                        return crow::response(
                            400,
                            "Confirmation Date is required.");
                    }


                    if (confirmedQuantity < 0)
                    {
                        return crow::response(
                            400,
                            "Confirmed Quantity cannot be negative.");
                    }


                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();


                    if (procurementManager.findOrder(id, materialID) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Procurement Order not found.");
                    }

                    // A confirmed quantity of 0 means this material
                    // line was never actually needed - delete it
                    // outright instead of "confirming" a zero
                    // quantity. Kept as its own branch here (rather
                    // than moved into confirmOrder) so a 0 always
                    // goes through the same receipts-guard as an
                    // explicit delete call.

                    if (confirmedQuantity == 0)
                    {
                        bool deleted =
                            procurementManager.deleteOrder(id, materialID);

                        if (!deleted)
                        {
                            return crow::response(
                                400,
                                "Could not delete Procurement Order line "
                                "(it may already have receipts recorded "
                                "against it, or already be confirmed).");
                        }

                        crow::json::wvalue response;

                        response["success"] = true;

                        response["deleted"] = true;

                        response["message"] =
                            "Procurement Order line deleted.";

                        return crow::response(response);
                    }


                    bool success =
                        procurementManager.confirmOrder(
                            id,
                            materialID,
                            confirmationDate,
                            confirmedQuantity);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Could not confirm Procurement Order.");
                    }


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Procurement Order confirmed successfully.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// PROCUREMENT - DELETE ORDER
// ============================================================
// Separate explicit endpoint for deleting an order outright, used
// by the "0 = delete" confirmation flow above and available for the
// UI to call directly. Refuses orders that already have receipts.

    CROW_ROUTE(app, "/api/procurement/delete")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    string id =
                        body["id"].s();

                    string materialID =
                        body["materialID"].s();


                    if (id.empty())
                    {
                        return crow::response(
                            400,
                            "Procurement Order ID is required.");
                    }


                    if (materialID.empty())
                    {
                        return crow::response(
                            400,
                            "Material ID is required.");
                    }


                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();


                    if (procurementManager.findOrder(id, materialID) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Procurement Order not found.");
                    }


                    bool success =
                        procurementManager.deleteOrder(id, materialID);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Could not delete Procurement Order "
                            "(it may already have receipts recorded "
                            "against it, or already be confirmed - "
                            "cancel or close it instead).");
                    }


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Procurement Order deleted.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// PROCUREMENT - RECEIVE (creates a partial or final receipt and
// performs the matching Goods Receipt)
// ============================================================

    CROW_ROUTE(app, "/api/procurement/receive")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);


                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    string id =
                        body["id"].s();

                    string materialID =
                        body["materialID"].s();

                    string receiptDate =
                        body["receiptDate"].s();

                    double receivedQuantity =
                        jsonQuantity(body["receivedQuantity"]);

                    {
                        string decimalsError =
                            quantityDecimalsError(receivedQuantity, "Received Quantity");

                        if (!decimalsError.empty())
                        {
                            return crow::response(
                                400,
                                decimalsError);
                        }
                    }

                    string comment = "";

                    if (body.has("comment"))
                    {
                        comment =
                            body["comment"].s();
                    }


                    if (id.empty())
                    {
                        return crow::response(
                            400,
                            "Procurement Order ID is required.");
                    }


                    if (materialID.empty())
                    {
                        return crow::response(
                            400,
                            "Material ID is required.");
                    }


                    if (receiptDate.empty())
                    {
                        return crow::response(
                            400,
                            "Receipt Date is required.");
                    }


                    if (receivedQuantity <= 0)
                    {
                        return crow::response(
                            400,
                            "Received Quantity must be greater than zero.");
                    }


                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();


                    ProcurementOrder* order =
                        procurementManager.findOrder(id, materialID);


                    if (order == nullptr)
                    {
                        return crow::response(
                            404,
                            "Procurement Order not found.");
                    }


                    if (order->getPendingQuantity() <= 0)
                    {
                        return crow::response(
                            409,
                            "This Procurement Order is already completed.");
                    }


                    {
                        string lockedError =
                            warehouseLockedError(
                                warehouseSystem,
                                order->getWarehouseID());

                        if (!lockedError.empty())
                        {
                            return crow::response(
                                409,
                                lockedError);
                        }
                    }

                    bool success =
                        procurementManager.receiveOrder(
                            id,
                            materialID,
                            receiptDate,
                            receivedQuantity,
                            comment);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Goods receipt failed. Check warehouse and material.");
                    }


                    // Receiving here also performs a Goods Receipt into
                    // the Warehouse (see ProcurementManager::
                    // receiveOrder()), which is Warehouse stock data,
                    // not Procurement's own - Procurement already saved
                    // its own order file above, but this needs the same
                    // auto-save check the direct Goods Receipt route
                    // uses.

                    autoSaveIfEnabled(warehouseSystem);


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Receipt recorded and goods received successfully.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });


// ============================================================
// PROCUREMENT ORDERS - CLOSE (mark as never arriving in full)
// ============================================================
// The supplier is not going to deliver the rest of this order (lost
// goods, discontinued item, cancelled after a partial delivery...).
// Closes every material line still pending under this "PRC-######"
// id - whatever was already received stays in inventory - and the
// whole order moves out of Confirmed Orders into Archived Orders,
// labeled "Closed (Incomplete)" instead of "Completed".

    CROW_ROUTE(app, "/api/procurement/close")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string id =
                        body["id"].s();

                    if (id.empty())
                    {
                        return crow::response(
                            400,
                            "Procurement Order ID is required.");
                    }

                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();

                    if (procurementManager.findOrder(id) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Procurement Order not found.");
                    }

                    bool success =
                        procurementManager.closeOrder(id);

                    if (!success)
                    {
                        return crow::response(
                            409,
                            "Nothing pending left to close on this order.");
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Order closed. Materials already received stay in "
                        "inventory; the rest no longer counts as pending.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });


// ============================================================
// PROCUREMENT - CANCEL ORDER
// ============================================================
// For an order still fully unconfirmed (Open Orders) that will
// never be confirmed. Moves it to Archived Orders as "Cancelled" -
// distinct from "Closed (Incomplete)", which is for an order that
// was confirmed and partially received before the rest stopped
// coming.

    CROW_ROUTE(app, "/api/procurement/cancel")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string id =
                        body["id"].s();

                    if (id.empty())
                    {
                        return crow::response(
                            400,
                            "Procurement Order ID is required.");
                    }

                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();

                    if (procurementManager.findOrder(id) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Procurement Order not found.");
                    }

                    bool success =
                        procurementManager.cancelOrder(id);

                    if (!success)
                    {
                        return crow::response(
                            409,
                            "Nothing left to cancel on this order (it may "
                            "already be confirmed).");
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Order cancelled and moved to Archived Orders.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });


// ============================================================
// MATERIAL IMAGES
// ============================================================

    CROW_ROUTE(app, "/web/images/materials/<string>")
        ([]
        (string filename)
            {
                string filepath =
                    "web/images/materials/" + filename;


                ifstream imageFile(
                    filepath,
                    ios::binary);


                if (!imageFile)
                {
                    return crow::response(
                        404,
                        "Image not found.");
                }


                string imageData(
                    (istreambuf_iterator<char>(
                        imageFile)),
                    istreambuf_iterator<char>()
                );


                crow::response response(
                    imageData);


                response.set_header(
                    "Content-Type",
                    "image/jpeg");


                return response;
            });

// ============================================================
// SET DATA LOGGING
// ============================================================

    CROW_ROUTE(app, "/api/data/logging")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }


                    bool enabled =
                        body["enabled"].b();


                    warehouseSystem->
                        setLogDataOperations(
                            enabled);


                    crow::json::wvalue response;

                    response["success"] =
                        true;

                    response["enabled"] =
                        enabled;

                    response["message"] =
                        enabled
                        ? "Save/Load logging enabled."
                        : "Save/Load logging disabled.";


                    return crow::response(
                        response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

// ============================================================
// GET DATA LOGGING STATUS
// ============================================================

    CROW_ROUTE(app, "/api/data/logging")
        .methods(crow::HTTPMethod::GET)
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                response["enabled"] =
                    warehouseSystem->getLogDataOperations();

                return crow::response(
                    response);
            });

    // ============================================================
    // PURCHASE - PENDING RECEIPTS (not yet priced)
    // ============================================================

    CROW_ROUTE(app, "/api/purchase/pending")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list pendingList;


                PurchaseManager& purchaseManager =
                    warehouseSystem->getPurchaseManager();

                ProcurementManager& procurementManager =
                    warehouseSystem->getProcurementManager();

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();


                for (const auto& pendingReceipt :
                    purchaseManager.getPendingReceipts())
                {
                    crow::json::wvalue item;

                    item["procurementOrderID"] =
                        pendingReceipt.procurementOrderID;

                    item["materialID"] =
                        pendingReceipt.materialID;

                    item["receiptIndex"] =
                        pendingReceipt.receiptIndex;

                    item["receiptDate"] =
                        pendingReceipt.receiptDate;

                    item["receivedQuantity"] =
                        pendingReceipt.receivedQuantity;

                    item["comment"] =
                        pendingReceipt.comment;


                    // Disambiguate by materialID: one "PRC-######" ID
                    // can be shared by several material lines.
                    ProcurementOrder* order =
                        procurementManager.findOrder(
                            pendingReceipt.procurementOrderID,
                            pendingReceipt.materialID);

                    item["productID"] =
                        (order != nullptr) ?
                        order->getProductID() : "";

                    item["warehouseID"] =
                        (order != nullptr) ?
                        order->getWarehouseID() : 0;


                    Material* material =
                        materialManager.findMaterial(
                            pendingReceipt.materialID);

                    item["materialName"] =
                        (material != nullptr) ?
                        material->getName() :
                        ((order != nullptr) ?
                            order->getMaterialName() : "");


                    Supplier* supplier =
                        (material != nullptr) ?
                        material->getSupplier() : nullptr;

                    item["supplierName"] =
                        (supplier != nullptr) ?
                        supplier->getName() : "No Supplier";


                    // The last price on record for this material, if
                    // any - shown alongside the cost entry field so a
                    // new price can be compared against the last one
                    // at a glance.
                    bool hasPreviousPrice =
                        purchaseManager.hasPriceFor(
                            pendingReceipt.materialID);

                    item["hasPreviousPrice"] =
                        hasPreviousPrice;

                    item["previousUnitPriceEUR"] =
                        hasPreviousPrice ?
                        purchaseManager.getCurrentUnitPriceEUR(
                            pendingReceipt.materialID) : 0.0;


                    pendingList.push_back(
                        std::move(item));
                }


                response["pending"] =
                    std::move(pendingList);

                return response;
            });

    // ============================================================
    // PURCHASE - LIST INVOICES
    // ============================================================

    CROW_ROUTE(app, "/api/purchase/invoices")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list invoiceList;


                PurchaseManager& purchaseManager =
                    warehouseSystem->getPurchaseManager();

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();


                for (const auto& invoice :
                    purchaseManager.getInvoices())
                {
                    crow::json::wvalue item;

                    item["id"] =
                        invoice->getID();

                    item["date"] =
                        invoice->getDate();

                    item["currency"] =
                        invoice->getCurrency();

                    item["exchangeRate"] =
                        invoice->getExchangeRate();

                    item["customsCost"] =
                        invoice->getCustomsCost();

                    item["freightCost"] =
                        invoice->getFreightCost();

                    item["comment"] =
                        invoice->getComment();

                    item["totalMaterialCost"] =
                        invoice->getTotalMaterialCost();


                    crow::json::wvalue::list lineList;

                    const auto& lines =
                        invoice->getLines();

                    for (size_t i = 0; i < lines.size(); i++)
                    {
                        crow::json::wvalue lineItem;

                        lineItem["procurementOrderID"] =
                            lines[i].procurementOrderID;

                        lineItem["materialID"] =
                            lines[i].materialID;

                        lineItem["receiptIndex"] =
                            lines[i].receiptIndex;

                        lineItem["receivedQuantity"] =
                            lines[i].receivedQuantity;

                        Material* lineMaterial =
                            materialManager.findMaterial(
                                lines[i].materialID);

                        Supplier* lineSupplier =
                            (lineMaterial != nullptr) ?
                            lineMaterial->getSupplier() : nullptr;

                        lineItem["supplierName"] =
                            (lineSupplier != nullptr) ?
                            lineSupplier->getName() : "";

                        lineItem["unitCost"] =
                            lines[i].unitCost;

                        lineItem["materialCost"] =
                            invoice->getLineMaterialCost(i);

                        lineItem["allocatedCost"] =
                            invoice->getLineAllocatedCost(i);

                        lineItem["unitPrice"] =
                            invoice->getLineUnitPrice(i);

                        lineItem["unitPriceEUR"] =
                            invoice->getLineUnitPriceEUR(i);

                        double otherCostAdditionEUR =
                            purchaseManager.getLineOtherCostAdditionEUR(
                                invoice.get(), i);

                        lineItem["otherCostAdditionEUR"] =
                            otherCostAdditionEUR;

                        // The total EUR value of the Other Cost
                        // addition on this line - per-unit addition
                        // times this delivery's own quantity, i.e.
                        // "how many euros of the one-time charge this
                        // invoice's line is carrying" (not the
                        // one-time charge's full original amount).
                        lineItem["otherCostTotalEUR"] =
                            otherCostAdditionEUR *
                            lines[i].receivedQuantity;

                        lineItem["landedUnitPriceEUR"] =
                            purchaseManager.getLineLandedUnitPriceEUR(
                                invoice.get(), i);

                        // Quotation this line was priced from, and how
                        // far the invoiced price (before customs /
                        // freight, like a quotation) is from it
                        lineItem["quotationID"] =
                            lines[i].quotationID;

                        lineItem["quotedUnitPriceEUR"] =
                            lines[i].quotedUnitPriceEUR;

                        double invoicedUnitEUR =
                            lines[i].unitCost *
                            invoice->getExchangeRate();

                        lineItem["invoicedUnitCostEUR"] =
                            invoicedUnitEUR;

                        bool linked =
                            !lines[i].quotationID.empty() &&
                            lines[i].quotedUnitPriceEUR > 0.0;

                        lineItem["deviationEUR"] =
                            linked ?
                            invoicedUnitEUR -
                                lines[i].quotedUnitPriceEUR : 0.0;

                        lineItem["deviationPct"] =
                            linked ?
                            (invoicedUnitEUR -
                                lines[i].quotedUnitPriceEUR) /
                                lines[i].quotedUnitPriceEUR * 100.0 : 0.0;

                        lineList.push_back(
                            std::move(lineItem));
                    }

                    item["lines"] =
                        std::move(lineList);


                    crow::json::wvalue::list otherCostList;

                    for (const auto& otherCost :
                        invoice->getOtherCosts())
                    {
                        crow::json::wvalue otherCostItem;

                        otherCostItem["supplierName"] =
                            otherCost.supplierName;

                        otherCostItem["amountEUR"] =
                            otherCost.amountEUR;

                        otherCostItem["supplierWide"] =
                            otherCost.supplierWide;

                        otherCostItem["comment"] =
                            otherCost.comment;

                        otherCostList.push_back(
                            std::move(otherCostItem));
                    }

                    item["otherCosts"] =
                        std::move(otherCostList);


                    // Supplier-wide Other Costs of earlier invoices
                    // that also land on this one

                    crow::json::wvalue::list inheritedList;

                    for (const auto& inherited :
                        purchaseManager.getInheritedOtherCosts(
                            invoice.get()))
                    {
                        crow::json::wvalue inheritedItem;

                        inheritedItem["sourceInvoiceID"] =
                            inherited.sourceInvoiceID;

                        inheritedItem["supplierName"] =
                            inherited.supplierName;

                        inheritedItem["amountEUR"] =
                            inherited.amountEUR;

                        inheritedItem["comment"] =
                            inherited.comment;

                        inheritedItem["quantityBasis"] =
                            inherited.quantityBasis;

                        inheritedItem["shareEUR"] =
                            inherited.shareEUR;

                        inheritedList.push_back(
                            std::move(inheritedItem));
                    }

                    item["inheritedOtherCosts"] =
                        std::move(inheritedList);


                    invoiceList.push_back(
                        std::move(item));
                }


                response["invoices"] =
                    std::move(invoiceList);

                return response;
            });

    // ============================================================
    // PURCHASE - CREATE INVOICE
    // ============================================================

    CROW_ROUTE(app, "/api/purchase/invoices/create")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string date =
                        body["date"].s();

                    string currency =
                        body.has("currency") ?
                        string(body["currency"].s()) : "EUR";

                    double exchangeRate =
                        body.has("exchangeRate") ?
                        body["exchangeRate"].d() : 1.0;

                    double customsCost =
                        body.has("customsCost") ?
                        body["customsCost"].d() : 0.0;

                    double freightCost =
                        body.has("freightCost") ?
                        body["freightCost"].d() : 0.0;

                    string comment =
                        body.has("comment") ?
                        string(body["comment"].s()) : "";


                    if (date.empty())
                    {
                        return crow::response(
                            400,
                            "Invoice Date is required.");
                    }

                    if (currency != "EUR" && currency != "USD")
                    {
                        return crow::response(
                            400,
                            "Currency must be EUR or USD.");
                    }

                    if (exchangeRate <= 0.0)
                    {
                        return crow::response(
                            400,
                            "Exchange Rate must be greater than zero.");
                    }

                    if (!body.has("lines"))
                    {
                        return crow::response(
                            400,
                            "At least one priced delivery is required.");
                    }


                    vector<PurchaseInvoiceLine> lines;

                    for (const auto& lineJson : body["lines"])
                    {
                        PurchaseInvoiceLine line;

                        line.procurementOrderID =
                            lineJson["procurementOrderID"].s();

                        line.materialID =
                            lineJson["materialID"].s();

                        line.receiptIndex =
                            lineJson["receiptIndex"].i();

                        line.receivedQuantity =
                            roundQty(jsonQuantity(lineJson["receivedQuantity"]));

                        line.unitCost =
                            lineJson["unitCost"].d();

                        line.quotationID =
                            invoiceLineQuotationID(lineJson);

                        string quotationError =
                            resolveLineQuotation(warehouseSystem, line);

                        if (!quotationError.empty())
                        {
                            return crow::response(400, quotationError);
                        }

                        lines.push_back(line);
                    }


                    vector<PurchaseInvoiceOtherCost> otherCosts;

                    if (body.has("otherCosts"))
                    {
                        for (const auto& otherCostJson :
                            body["otherCosts"])
                        {
                            PurchaseInvoiceOtherCost otherCost;

                            otherCost.supplierName =
                                otherCostJson.has("supplierName") ?
                                string(otherCostJson["supplierName"].s()) : "";

                            otherCost.amountEUR =
                                otherCostJson["amountEUR"].d();

                            otherCost.supplierWide =
                                otherCostJson.has("supplierWide") &&
                                otherCostJson["supplierWide"].b();

                            otherCost.comment =
                                otherCostJson.has("comment") ?
                                string(otherCostJson["comment"].s()) : "";

                            otherCosts.push_back(otherCost);
                        }
                    }


                    // A Manual Price Adjustment line (procurementOrderID
                    // "MANUAL") has no Procurement receipt to validate it
                    // against, so - unlike a real delivery line, which
                    // createInvoice() already checks against Pending
                    // Pricing - its Material has to be checked here.

                    MaterialManager& materialManager =
                        warehouseSystem->getMaterialManager();

                    for (const auto& line : lines)
                    {
                        if (line.procurementOrderID == "MANUAL" &&
                            materialManager.findMaterial(
                                line.materialID) == nullptr)
                        {
                            return crow::response(
                                404,
                                "Material not found: " +
                                line.materialID);
                        }
                    }


                    PurchaseManager& purchaseManager =
                        warehouseSystem->getPurchaseManager();

                    string errorMessage;

                    PurchaseInvoice* invoice =
                        purchaseManager.createInvoice(
                            date,
                            currency,
                            exchangeRate,
                            customsCost,
                            freightCost,
                            comment,
                            lines,
                            otherCosts,
                            errorMessage);

                    if (invoice == nullptr)
                    {
                        return crow::response(
                            400,
                            errorMessage);
                    }


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["id"] = invoice->getID();

                    response["message"] =
                        "Purchase Invoice created successfully.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // ============================================================
    // PURCHASE - MANUAL PRICE ADJUSTMENT (no Procurement delivery)
    // ============================================================
    // For a Material that needs its price set or corrected directly -
    // e.g. a price known from another source, or a one-off fix - with
    // no received delivery behind it. Internally this is still a
    // one-line Purchase Invoice (so it shows up in the same Invoices
    // list and price history), just with no customs/freight and a
    // line marked as manual (see PurchaseInvoiceLine).

    CROW_ROUTE(app, "/api/purchase/materials/adjust")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string materialID =
                        body["materialID"].s();

                    string date =
                        body["date"].s();

                    string currency =
                        body.has("currency") ?
                        string(body["currency"].s()) : "EUR";

                    double exchangeRate =
                        body.has("exchangeRate") ?
                        body["exchangeRate"].d() : 1.0;

                    double unitCost =
                        body["unitCost"].d();

                    string comment =
                        body.has("comment") ?
                        string(body["comment"].s()) : "";


                    if (materialID.empty())
                    {
                        return crow::response(
                            400,
                            "Material is required.");
                    }

                    MaterialManager& materialManager =
                        warehouseSystem->getMaterialManager();

                    if (materialManager.findMaterial(materialID) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Material not found: " + materialID);
                    }

                    if (date.empty())
                    {
                        return crow::response(
                            400,
                            "Date is required.");
                    }

                    if (currency != "EUR" && currency != "USD")
                    {
                        return crow::response(
                            400,
                            "Currency must be EUR or USD.");
                    }

                    if (exchangeRate <= 0.0)
                    {
                        return crow::response(
                            400,
                            "Exchange Rate must be greater than zero.");
                    }

                    if (unitCost < 0.0)
                    {
                        return crow::response(
                            400,
                            "Unit Cost cannot be negative.");
                    }


                    PurchaseInvoiceLine line;

                    line.procurementOrderID = "MANUAL";
                    line.materialID = materialID;
                    line.receiptIndex = -1;
                    line.receivedQuantity = 1;
                    line.unitCost = unitCost;

                    vector<PurchaseInvoiceLine> lines;

                    lines.push_back(line);


                    PurchaseManager& purchaseManager =
                        warehouseSystem->getPurchaseManager();

                    string errorMessage;

                    PurchaseInvoice* invoice =
                        purchaseManager.createInvoice(
                            date,
                            currency,
                            exchangeRate,
                            0.0,
                            0.0,
                            comment.empty() ?
                                "Manual price adjustment" : comment,
                            lines,
                            vector<PurchaseInvoiceOtherCost>(),
                            errorMessage);

                    if (invoice == nullptr)
                    {
                        return crow::response(
                            400,
                            errorMessage);
                    }


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["id"] = invoice->getID();

                    response["unitPriceEUR"] =
                        invoice->getLineUnitPriceEUR(0);

                    response["message"] =
                        "Material price updated.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // ============================================================
    // PURCHASE - DELETE INVOICE
    // ============================================================

    CROW_ROUTE(app, "/api/purchase/invoices/delete")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string id =
                        body["id"].s();

                    if (id.empty())
                    {
                        return crow::response(
                            400,
                            "Purchase Invoice ID is required.");
                    }

                    PurchaseManager& purchaseManager =
                        warehouseSystem->getPurchaseManager();

                    if (purchaseManager.findInvoice(id) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Purchase Invoice not found.");
                    }

                    purchaseManager.deleteInvoice(id);

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Purchase Invoice deleted. Its deliveries are "
                        "pending again.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // ============================================================
    // PURCHASE - UPDATE INVOICE (rework costs after the fact - e.g.
    // the customs bill arrives separately from the supplier invoice)
    // ============================================================

    CROW_ROUTE(app, "/api/purchase/invoices/update")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string id =
                        body["id"].s();

                    if (id.empty())
                    {
                        return crow::response(
                            400,
                            "Purchase Invoice ID is required.");
                    }

                    string date =
                        body["date"].s();

                    string currency =
                        body.has("currency") ?
                        string(body["currency"].s()) : "EUR";

                    double exchangeRate =
                        body.has("exchangeRate") ?
                        body["exchangeRate"].d() : 1.0;

                    double customsCost =
                        body.has("customsCost") ?
                        body["customsCost"].d() : 0.0;

                    double freightCost =
                        body.has("freightCost") ?
                        body["freightCost"].d() : 0.0;

                    string comment =
                        body.has("comment") ?
                        string(body["comment"].s()) : "";

                    if (!body.has("lines"))
                    {
                        return crow::response(
                            400,
                            "Lines are required.");
                    }

                    vector<PurchaseInvoiceLine> lines;

                    for (const auto& lineJson : body["lines"])
                    {
                        PurchaseInvoiceLine line;

                        line.procurementOrderID =
                            lineJson["procurementOrderID"].s();

                        line.materialID =
                            lineJson["materialID"].s();

                        line.receiptIndex =
                            lineJson["receiptIndex"].i();

                        line.receivedQuantity =
                            roundQty(jsonQuantity(lineJson["receivedQuantity"]));

                        line.unitCost =
                            lineJson["unitCost"].d();

                        line.quotationID =
                            invoiceLineQuotationID(lineJson);

                        // Same quotation as before: keep the price
                        // frozen when the link was made

                        bool keptQuote = false;

                        PurchaseInvoice* existingInvoice =
                            warehouseSystem->getPurchaseManager()
                                .findInvoice(id);

                        if (existingInvoice != nullptr &&
                            lines.size() <
                                existingInvoice->getLines().size())
                        {
                            const PurchaseInvoiceLine& before =
                                existingInvoice->getLines()[lines.size()];

                            if (!line.quotationID.empty() &&
                                before.quotationID == line.quotationID)
                            {
                                line.quotedUnitPriceEUR =
                                    before.quotedUnitPriceEUR;

                                keptQuote = true;
                            }
                        }

                        if (!keptQuote)
                        {
                            string quotationError =
                                resolveLineQuotation(
                                    warehouseSystem, line);

                            if (!quotationError.empty())
                            {
                                return crow::response(
                                    400, quotationError);
                            }
                        }

                        lines.push_back(line);
                    }

                    vector<PurchaseInvoiceOtherCost> otherCosts;

                    if (body.has("otherCosts"))
                    {
                        for (const auto& otherCostJson :
                            body["otherCosts"])
                        {
                            PurchaseInvoiceOtherCost otherCost;

                            otherCost.supplierName =
                                otherCostJson.has("supplierName") ?
                                string(otherCostJson["supplierName"].s()) : "";

                            otherCost.amountEUR =
                                otherCostJson["amountEUR"].d();

                            otherCost.supplierWide =
                                otherCostJson.has("supplierWide") &&
                                otherCostJson["supplierWide"].b();

                            otherCost.comment =
                                otherCostJson.has("comment") ?
                                string(otherCostJson["comment"].s()) : "";

                            otherCosts.push_back(otherCost);
                        }
                    }

                    PurchaseManager& purchaseManager =
                        warehouseSystem->getPurchaseManager();

                    string errorMessage;

                    bool success =
                        purchaseManager.updateInvoice(
                            id,
                            date,
                            currency,
                            exchangeRate,
                            customsCost,
                            freightCost,
                            comment,
                            lines,
                            otherCosts,
                            errorMessage);

                    if (!success)
                    {
                        return crow::response(
                            400,
                            errorMessage);
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Purchase Invoice updated.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // ============================================================
    // PURCHASE - MATERIAL PRICE HISTORY
    // ============================================================

    CROW_ROUTE(app, "/api/purchase/materials/<string>/history")
        ([warehouseSystem]
        (string materialID)
            {
                crow::json::wvalue response;

                PurchaseManager& purchaseManager =
                    warehouseSystem->getPurchaseManager();

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();


                Material* material =
                    materialManager.findMaterial(materialID);

                response["materialID"] = materialID;

                response["materialName"] =
                    (material != nullptr) ?
                    material->getName() : "";

                response["currentUnitPriceEUR"] =
                    purchaseManager.getCurrentUnitPriceEUR(materialID);


                crow::json::wvalue::list historyList;

                for (const auto& entry :
                    purchaseManager.getPriceHistory(materialID))
                {
                    crow::json::wvalue item;

                    item["date"] = entry.date;
                    item["unitPriceEUR"] = entry.unitPriceEUR;
                    item["originalCurrency"] = entry.originalCurrency;
                    item["originalUnitPrice"] = entry.originalUnitPrice;
                    item["exchangeRateUsed"] = entry.exchangeRateUsed;
                    item["sourceInvoiceID"] = entry.sourceInvoiceID;
                    item["sourceProcurementOrderID"] =
                        entry.sourceProcurementOrderID;

                    historyList.push_back(
                        std::move(item));
                }

                response["history"] =
                    std::move(historyList);

                return response;
            });

    // ============================================================
    // PURCHASE - PRODUCT COST (all Products)
    // ============================================================

    CROW_ROUTE(app, "/api/purchase/products/cost")
        ([warehouseSystem](const crow::request& req)
            {
                // ?quantity=N: how many Products are produced (picks
                // the price tier of each material), ?date=: cost on a
                // past date. See CostEstimator.h.
                double heaterQuantity = 1.0;

                string asOfDate;

                costQueryParams(req, heaterQuantity, asOfDate);

                crow::json::wvalue response;

                crow::json::wvalue::list productList;


                PurchaseManager& purchaseManager =
                    warehouseSystem->getPurchaseManager();

                ProductManager& productManager =
                    warehouseSystem->getProductManager();

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();


                for (const auto& product :
                    productManager.getProducts())
                {
                    EstimatedProductCost cost =
                        computeEstimatedProductCost(
                            product->getID(),
                            product->getBOM(),
                            heaterQuantity,
                            asOfDate,
                            purchaseManager,
                            warehouseSystem->getQuotationManager(),
                            materialManager,
                            usualBasis(req));

                    crow::json::wvalue item;

                    item["realCostEUR"] = cost.realCostEUR;

                    item["estimatedCostEUR"] = cost.estimatedCostEUR;

                    item["hasEstimates"] = cost.hasEstimates;

                    item["productID"] = cost.productID;

                    item["productName"] =
                        product->getName();

                    item["totalCostEUR"] = cost.totalCostEUR;

                    item["complete"] = cost.complete;


                    crow::json::wvalue::list lineList;

                    for (const auto& line : cost.lines)
                    {
                        crow::json::wvalue lineItem;

                        lineItem["materialID"] = line.materialID;
                        lineItem["quantity"] = line.quantity;
                        lineItem["totalQuantity"] = line.totalQuantity;
                        lineItem["estimate"] =
                            estimateToJson(line.estimate);
                        lineItem["hasPrice"] = line.hasPrice;
                        lineItem["unitPriceEUR"] = line.unitPriceEUR;
                        lineItem["lineCostEUR"] = line.lineCostEUR;

                        // See the single-Product cost route below for
                        // why this falls back to "Other".

                        Material* lineMaterial =
                            materialManager.findMaterial(
                                line.materialID);

                        string category =
                            (lineMaterial != nullptr &&
                             !lineMaterial->getCategory().empty()) ?
                            lineMaterial->getCategory() : "Other";

                        lineItem["category"] = category;

                        lineItem["materialName"] =
                            lineMaterial != nullptr ?
                            lineMaterial->getName() : "";

                        lineList.push_back(
                            std::move(lineItem));
                    }

                    item["lines"] =
                        std::move(lineList);


                    productList.push_back(
                        std::move(item));
                }


                response["products"] =
                    std::move(productList);

                return response;
            });

    // ============================================================
    // PURCHASE - PRODUCT COST (single Product)
    // ============================================================

    CROW_ROUTE(app, "/api/purchase/products/<string>/cost")
        ([warehouseSystem]
        (const crow::request& req, string productID)
            {
                double heaterQuantity = 1.0;

                string asOfDate;

                costQueryParams(req, heaterQuantity, asOfDate);

                PurchaseManager& purchaseManager =
                    warehouseSystem->getPurchaseManager();

                ProductManager& productManager =
                    warehouseSystem->getProductManager();

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();


                Product* product =
                    productManager.findProduct(productID);

                if (product == nullptr)
                {
                    return crow::response(
                        404,
                        "Product not found.");
                }


                EstimatedProductCost cost =
                    computeEstimatedProductCost(
                        product->getID(),
                        product->getBOM(),
                        heaterQuantity,
                        asOfDate,
                        purchaseManager,
                        warehouseSystem->getQuotationManager(),
                        materialManager,
                        usualBasis(req));

                crow::json::wvalue response;

                response["realCostEUR"] = cost.realCostEUR;

                response["estimatedCostEUR"] = cost.estimatedCostEUR;

                response["hasEstimates"] = cost.hasEstimates;

                response["productID"] = cost.productID;

                response["productName"] =
                    product->getName();

                response["totalCostEUR"] = cost.totalCostEUR;

                response["complete"] = cost.complete;


                crow::json::wvalue::list lineList;

                for (const auto& line : cost.lines)
                {
                    crow::json::wvalue lineItem;

                    lineItem["materialID"] = line.materialID;
                    lineItem["quantity"] = line.quantity;
                    lineItem["totalQuantity"] = line.totalQuantity;
                    lineItem["estimate"] =
                        estimateToJson(line.estimate);
                    lineItem["hasPrice"] = line.hasPrice;
                    lineItem["unitPriceEUR"] = line.unitPriceEUR;
                    lineItem["lineCostEUR"] = line.lineCostEUR;

                    // Used to group this Product's cost by Category
                    // (see "Cost by Category" in the Purchase module) -
                    // a Material with no Category set falls back to
                    // "Other" rather than showing up blank.

                    Material* lineMaterial =
                        materialManager.findMaterial(line.materialID);

                    string category =
                        (lineMaterial != nullptr &&
                         !lineMaterial->getCategory().empty()) ?
                        lineMaterial->getCategory() : "Other";

                    lineItem["category"] = category;

                    lineItem["materialName"] =
                        lineMaterial != nullptr ?
                        lineMaterial->getName() : "";

                    lineList.push_back(
                        std::move(lineItem));
                }

                response["lines"] =
                    std::move(lineList);

                return crow::response(response);
            });

    // ============================================================
    // PURCHASE - EXCHANGE RATE CONFIG (GET)
    // ============================================================

    CROW_ROUTE(app, "/api/purchase/config")
        .methods(crow::HTTPMethod::GET)
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                response["exchangeRate"] =
                    warehouseSystem->getPurchaseManager()
                        .getDefaultExchangeRate();

                return response;
            });

    // ============================================================
    // PURCHASE - EXCHANGE RATE CONFIG (SET)
    // ============================================================

    CROW_ROUTE(app, "/api/purchase/config")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    double exchangeRate =
                        body["exchangeRate"].d();

                    PurchaseManager& purchaseManager =
                        warehouseSystem->getPurchaseManager();

                    bool success =
                        purchaseManager.setDefaultExchangeRate(
                            exchangeRate);

                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Exchange Rate must be greater than zero.");
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["exchangeRate"] = exchangeRate;

                    response["message"] =
                        "Exchange rate updated.";

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // ============================================================
    // STATUS
    // ============================================================

    CROW_ROUTE(app, "/api/status")
        ([]()
            {
                crow::json::wvalue response;

                response["status"] = "online";

                response["message"] =
                    "Warehouse Management System is running.";

                return response;
            });

// ============================================================
// PROCUREMENT - ESTIMATED RECEIPT DATE (optional, one per order)
// ============================================================
// Body: { id, estimatedReceiptDate }
// One date for the whole "PRC-######" (applied to every line still
// waiting for goods). An empty date clears it. Informational only.

    CROW_ROUTE(app, "/api/procurement/estimated-date")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body =
                        crow::json::load(req.body);

                    if (!body ||
                        !body.has("id") ||
                        !body.has("estimatedReceiptDate"))
                    {
                        return crow::response(
                            400,
                            "Order ID and date are required.");
                    }

                    string id =
                        body["id"].s();

                    string date =
                        body["estimatedReceiptDate"].s();


                    // Empty = clear. Otherwise a plain YYYY-MM-DD.

                    if (!date.empty())
                    {
                        bool validFormat =
                            date.size() == 10 &&
                            date[4] == '-' &&
                            date[7] == '-';

                        for (size_t i = 0;
                            validFormat && i < date.size();
                            i++)
                        {
                            if (i == 4 || i == 7)
                            {
                                continue;
                            }

                            if (!isdigit(
                                static_cast<unsigned char>(date[i])))
                            {
                                validFormat = false;
                            }
                        }

                        if (!validFormat)
                        {
                            return crow::response(
                                400,
                                "Estimated Delivery must be a date "
                                "(YYYY-MM-DD).");
                        }
                    }


                    ProcurementManager& procurementManager =
                        warehouseSystem->getProcurementManager();

                    vector<ProcurementOrder*> orderLines =
                        procurementManager.findOrderLines(id);

                    if (orderLines.empty())
                    {
                        return crow::response(
                            404,
                            "Procurement Order not found.");
                    }

                    // ISO dates compare correctly as plain strings.

                    if (!date.empty())
                    {
                        for (ProcurementOrder* line : orderLines)
                        {
                            if (date < line->getOrderDate())
                            {
                                return crow::response(
                                    400,
                                    "Estimated Delivery cannot be before "
                                    "the Order Date ("
                                    + line->getOrderDate() + ").");
                            }
                        }
                    }

                    if (!procurementManager.setEstimatedReceiptDate(
                        id,
                        date))
                    {
                        return crow::response(
                            400,
                            "This order is already completed, closed or "
                            "cancelled - its Estimated Delivery can no "
                            "longer be changed.");
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    return crow::response(response);
                }

                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });


// ============================================================
// LOGIN / USERS (see AuthManager.h)
// ============================================================
// Who may call what is decided before any route runs, by
// AuthMiddleware -> AuthManager::decide(). The routes below only
// do the account work itself.

    CROW_ROUTE(app, "/api/auth/login")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req)
            {
                auto body =
                    crow::json::load(req.body);

                if (!body ||
                    !body.has("username") ||
                    !body.has("password"))
                {
                    return crow::response(
                        400,
                        "Username and password are required.");
                }

                string username =
                    body["username"].s();

                string password =
                    body["password"].s();

                string token;

                auth::CurrentUser user;

                string problem =
                    auth::AuthManager::instance().login(
                        username,
                        password,
                        token,
                        user);

                if (!problem.empty())
                {
                    return crow::response(
                        401,
                        problem);
                }

                crow::json::wvalue result;

                result["username"] = user.username;

                result["role"] =
                    auth::roleToString(user.role);

                crow::response response(result);

                response.set_header(
                    "Set-Cookie",
                    "wms_session=" + token +
                    "; Path=/; HttpOnly; SameSite=Strict; Max-Age=43200");

                return response;
            });


    CROW_ROUTE(app, "/api/auth/logout")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req)
            {
                string token =
                    auth::AuthManager::cookieValue(
                        req.get_header_value("Cookie"),
                        "wms_session");

                auth::AuthManager::instance().logout(token);

                crow::json::wvalue result;

                result["success"] = true;

                crow::response response(result);

                response.set_header(
                    "Set-Cookie",
                    "wms_session=; Path=/; HttpOnly; SameSite=Strict; Max-Age=0");

                return response;
            });


    CROW_ROUTE(app, "/api/auth/me")
        ([](const crow::request& req)
            {
                string token =
                    auth::AuthManager::cookieValue(
                        req.get_header_value("Cookie"),
                        "wms_session");

                auth::CurrentUser user;

                if (!auth::AuthManager::instance()
                    .userForToken(token, user))
                {
                    return crow::response(
                        401,
                        "Please log in.");
                }

                crow::json::wvalue result;

                result["username"] = user.username;

                result["role"] =
                    auth::roleToString(user.role);

                return crow::response(result);
            });


    CROW_ROUTE(app, "/api/auth/change-password")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req)
            {
                auto body =
                    crow::json::load(req.body);

                if (!body ||
                    !body.has("currentPassword") ||
                    !body.has("newPassword"))
                {
                    return crow::response(
                        400,
                        "Current and new password are required.");
                }

                string token =
                    auth::AuthManager::cookieValue(
                        req.get_header_value("Cookie"),
                        "wms_session");

                auth::CurrentUser user;

                if (!auth::AuthManager::instance()
                    .userForToken(token, user))
                {
                    return crow::response(
                        401,
                        "Please log in.");
                }

                string currentPassword =
                    body["currentPassword"].s();

                string newPassword =
                    body["newPassword"].s();

                string problem =
                    auth::AuthManager::instance()
                    .changeOwnPassword(
                        user.username,
                        currentPassword,
                        newPassword);

                if (!problem.empty())
                {
                    return crow::response(
                        400,
                        problem);
                }

                // Changing a password closes every session of this
                // user (including this one) - log in again.

                crow::json::wvalue result;

                result["success"] = true;

                result["message"] =
                    "Password changed. Please log in again.";

                crow::response response(result);

                response.set_header(
                    "Set-Cookie",
                    "wms_session=; Path=/; HttpOnly; SameSite=Strict; Max-Age=0");

                return response;
            });


    CROW_ROUTE(app, "/api/users")
        ([]()
            {
                crow::json::wvalue result;

                crow::json::wvalue::list userList;

                for (const auto& user :
                    auth::AuthManager::instance().listUsers())
                {
                    crow::json::wvalue item;

                    item["username"] = user.username;

                    item["role"] =
                        auth::roleToString(user.role);

                    userList.push_back(std::move(item));
                }

                result["users"] = std::move(userList);

                return crow::response(result);
            });


    CROW_ROUTE(app, "/api/users/create")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req)
            {
                auto body =
                    crow::json::load(req.body);

                if (!body ||
                    !body.has("username") ||
                    !body.has("password") ||
                    !body.has("role"))
                {
                    return crow::response(
                        400,
                        "Username, password and role are required.");
                }

                string username =
                    body["username"].s();

                string password =
                    body["password"].s();

                string role =
                    body["role"].s();

                string problem =
                    auth::AuthManager::instance()
                    .createUser(username, password, role);

                if (!problem.empty())
                {
                    return crow::response(
                        400,
                        problem);
                }

                crow::json::wvalue result;

                result["success"] = true;

                return crow::response(result);
            });


    CROW_ROUTE(app, "/api/users/role")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req)
            {
                auto body =
                    crow::json::load(req.body);

                if (!body ||
                    !body.has("username") ||
                    !body.has("role"))
                {
                    return crow::response(
                        400,
                        "Username and role are required.");
                }

                string username =
                    body["username"].s();

                string role =
                    body["role"].s();

                string problem =
                    auth::AuthManager::instance()
                    .setRole(username, role);

                if (!problem.empty())
                {
                    return crow::response(
                        400,
                        problem);
                }

                crow::json::wvalue result;

                result["success"] = true;

                return crow::response(result);
            });


    CROW_ROUTE(app, "/api/users/reset-password")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req)
            {
                auto body =
                    crow::json::load(req.body);

                if (!body ||
                    !body.has("username") ||
                    !body.has("newPassword"))
                {
                    return crow::response(
                        400,
                        "Username and new password are required.");
                }

                string username =
                    body["username"].s();

                string newPassword =
                    body["newPassword"].s();

                string problem =
                    auth::AuthManager::instance()
                    .setPassword(username, newPassword);

                if (!problem.empty())
                {
                    return crow::response(
                        400,
                        problem);
                }

                crow::json::wvalue result;

                result["success"] = true;

                return crow::response(result);
            });


    CROW_ROUTE(app, "/api/users/delete")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req)
            {
                auto body =
                    crow::json::load(req.body);

                if (!body || !body.has("username"))
                {
                    return crow::response(
                        400,
                        "Username is required.");
                }

                string token =
                    auth::AuthManager::cookieValue(
                        req.get_header_value("Cookie"),
                        "wms_session");

                auth::CurrentUser actingUser;

                auth::AuthManager::instance()
                    .userForToken(token, actingUser);

                string username =
                    body["username"].s();

                string problem =
                    auth::AuthManager::instance()
                    .deleteUser(username, actingUser.username);

                if (!problem.empty())
                {
                    return crow::response(
                        400,
                        problem);
                }

                crow::json::wvalue result;

                result["success"] = true;

                return crow::response(result);
            });


    // ============================================================
    // INVENTORY COUNTS (physical inventory of one Warehouse)
    // ============================================================
    // See InventoryCountManager.h. A count stays Open (draft, stock
    // untouched, Warehouse locked) until it is applied or cancelled.

    // List of all counts, newest first, plus the available reasons

    CROW_ROUTE(app, "/api/inventory-counts")
        .methods(crow::HTTPMethod::GET)
        ([warehouseSystem]()
            {
                InventoryCountManager& countManager =
                    warehouseSystem->getInventoryCountManager();

                WarehouseManager& warehouseManager =
                    warehouseSystem->getWarehouseManager();

                crow::json::wvalue response;

                crow::json::wvalue::list countList;

                const auto& counts = countManager.getCounts();

                for (size_t i = counts.size(); i > 0; i--)
                {
                    const InventoryCount& count = *counts[i - 1];

                    crow::json::wvalue item;

                    item["id"] = count.id;

                    item["warehouseID"] = count.warehouseID;

                    Warehouse* warehouse =
                        warehouseManager.findWarehouse(
                            count.warehouseID);

                    item["warehouseName"] =
                        (warehouse != nullptr) ?
                        warehouse->getName() : "";

                    item["status"] = count.status;

                    item["startDate"] = count.startDate;

                    item["endDate"] = count.endDate;

                    item["comment"] = count.comment;

                    int counted = 0;

                    int adjusted = 0;

                    for (const auto& line : count.lines)
                    {
                        if (line.counted)
                        {
                            counted++;

                            if (count.status == "Applied" &&
                                line.difference != 0.0)
                            {
                                adjusted++;
                            }
                        }
                    }

                    item["lineCount"] =
                        static_cast<int>(count.lines.size());

                    item["countedCount"] = counted;

                    item["pendingCount"] =
                        static_cast<int>(count.lines.size()) - counted;

                    item["adjustedCount"] = adjusted;

                    countList.push_back(std::move(item));
                }

                response["counts"] = std::move(countList);

                crow::json::wvalue::list reasonList;

                for (const string& reason :
                    InventoryCountManager::reasons())
                {
                    reasonList.push_back(reason);
                }

                response["reasons"] = std::move(reasonList);

                return response;
            });


    // One count with its lines

    CROW_ROUTE(app, "/api/inventory-counts/<string>")
        .methods(crow::HTTPMethod::GET)
        ([warehouseSystem](const string& id)
            {
                InventoryCountManager& countManager =
                    warehouseSystem->getInventoryCountManager();

                InventoryCount* count =
                    countManager.findCount(id);

                if (count == nullptr)
                {
                    return crow::response(
                        404,
                        "Inventory count not found.");
                }

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();

                Warehouse* warehouse =
                    warehouseSystem->getWarehouseManager()
                    .findWarehouse(count->warehouseID);

                crow::json::wvalue response;

                response["id"] = count->id;

                response["warehouseID"] = count->warehouseID;

                response["warehouseName"] =
                    (warehouse != nullptr) ?
                    warehouse->getName() : "";

                response["status"] = count->status;

                response["startDate"] = count->startDate;

                response["endDate"] = count->endDate;

                response["comment"] = count->comment;

                bool isOpen = (count->status == "Open");

                crow::json::wvalue::list lineList;

                for (const auto& line : count->lines)
                {
                    crow::json::wvalue item;

                    Material* material =
                        materialManager.findMaterial(line.materialID);

                    item["materialID"] = line.materialID;

                    item["materialName"] =
                        (material != nullptr) ?
                        material->getName() : "";

                    item["uom"] =
                        (material != nullptr) ?
                        material->getUoM() : "";

                    item["photo"] =
                        (material != nullptr) ?
                        material->getPhotoPath() : "";

                    item["internalSupplier"] =
                        (material != nullptr) &&
                        material->hasInternalSupplier();

                    // While Open the system quantity is the live stock
                    // (it cannot change - the Warehouse is locked);
                    // once Applied it is the quantity recorded then.
                    double system =
                        isOpen ?
                        countManager.getSystemQuantity(
                            count->warehouseID, line.materialID) :
                        line.systemQuantity;

                    item["systemQuantity"] = system;

                    item["counted"] = line.counted;

                    item["countedQuantity"] =
                        line.counted ? line.countedQuantity : 0.0;

                    item["difference"] =
                        line.counted ?
                        roundQty(line.countedQuantity - system) : 0.0;

                    item["reason"] = line.reason;

                    item["comment"] = line.comment;

                    lineList.push_back(std::move(item));
                }

                response["lines"] = std::move(lineList);

                crow::json::wvalue::list reasonList;

                for (const string& reason :
                    InventoryCountManager::reasons())
                {
                    reasonList.push_back(reason);
                }

                response["reasons"] = std::move(reasonList);

                return crow::response(response);
            });


    // Open a new count: body { warehouseID, startDate, comment }

    CROW_ROUTE(app, "/api/inventory-counts/create")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    int warehouseID =
                        body["warehouseID"].i();

                    string startDate =
                        body["startDate"].s();

                    string comment = "";

                    if (body.has("comment"))
                    {
                        comment = body["comment"].s();
                    }

                    string error;

                    InventoryCount* count =
                        warehouseSystem->getInventoryCountManager()
                        .createCount(
                            warehouseID,
                            startDate,
                            comment,
                            error);

                    if (count == nullptr)
                    {
                        return crow::response(400, error);
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["id"] = count->id;

                    response["message"] =
                        "Inventory count " + count->id +
                        " opened. Stock movements in Warehouse " +
                        to_string(warehouseID) +
                        " are suspended until it is applied or "
                        "cancelled.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        400,
                        string("Invalid request: ") + e.what());
                }
            });


    // Save the draft:
    // body { id, comment, lines: [ { materialID, counted,
    //        countedQuantity, reason, comment } ] }

    CROW_ROUTE(app, "/api/inventory-counts/save")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string id = body["id"].s();

                    string comment = "";

                    if (body.has("comment"))
                    {
                        comment = body["comment"].s();
                    }

                    vector<InventoryCountLine> lines;

                    if (body.has("lines"))
                    {
                        for (const auto& lineJson : body["lines"])
                        {
                            InventoryCountLine line;

                            line.materialID =
                                lineJson["materialID"].s();

                            line.counted =
                                lineJson.has("counted") &&
                                lineJson["counted"].b();

                            if (line.counted)
                            {
                                line.countedQuantity =
                                    jsonQuantity(
                                        lineJson["countedQuantity"]);
                            }

                            if (lineJson.has("reason"))
                            {
                                line.reason =
                                    lineJson["reason"].s();
                            }

                            if (lineJson.has("comment"))
                            {
                                line.comment =
                                    lineJson["comment"].s();
                            }

                            lines.push_back(line);
                        }
                    }

                    string error;

                    if (!warehouseSystem->getInventoryCountManager()
                        .saveDraft(id, comment, lines, error))
                    {
                        return crow::response(400, error);
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] = "Inventory count saved.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        400,
                        string("Invalid request: ") + e.what());
                }
            });


    // Apply: body { id, endDate }

    CROW_ROUTE(app, "/api/inventory-counts/apply")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string id = body["id"].s();

                    string endDate = body["endDate"].s();

                    InventoryCountSummary summary;

                    string error;

                    if (!warehouseSystem->getInventoryCountManager()
                        .applyCount(id, endDate, summary, error))
                    {
                        return crow::response(400, error);
                    }

                    // Stock changed - same auto save as any other
                    // stock movement.
                    autoSaveIfEnabled(warehouseSystem);

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["linesCounted"] = summary.linesCounted;

                    response["linesAdjusted"] = summary.linesAdjusted;

                    response["linesPending"] = summary.linesPending;

                    response["message"] =
                        "Inventory count " + id + " applied: " +
                        to_string(summary.linesAdjusted) +
                        " material(s) adjusted, " +
                        to_string(summary.linesCounted -
                            summary.linesAdjusted) +
                        " counted without difference, " +
                        to_string(summary.linesPending) +
                        " not counted.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        400,
                        string("Invalid request: ") + e.what());
                }
            });


    // Cancel: body { id }

    CROW_ROUTE(app, "/api/inventory-counts/cancel")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(
                            400,
                            "Invalid JSON data.");
                    }

                    string id = body["id"].s();

                    string error;

                    if (!warehouseSystem->getInventoryCountManager()
                        .cancelCount(id, error))
                    {
                        return crow::response(400, error);
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Inventory count " + id + " cancelled. "
                        "Stock was not changed.";

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        400,
                        string("Invalid request: ") + e.what());
                }
            });


    // ============================================================
    // MOVEMENT LEDGER (history of everything in the Warehouse)
    // ============================================================
    // GET /api/movements?material=ID&product=ID&warehouse=N
    //     &type=TEXT&from=YYYY-MM-DD&to=YYYY-MM-DD&limit=N
    // Newest first. Every filter is optional.

    CROW_ROUTE(app, "/api/movements")
        .methods(crow::HTTPMethod::GET)
        ([warehouseSystem](const crow::request& req)
            {
                auto param = [&](const char* key) -> string
                {
                    const char* value = req.url_params.get(key);

                    return value != nullptr ? string(value) : string();
                };

                string material = param("material");

                string product = param("product");

                string warehouse = param("warehouse");

                string type = param("type");

                string from = param("from");

                string to = param("to");

                size_t limit = 5000;

                if (!param("limit").empty())
                {
                    try
                    {
                        limit = static_cast<size_t>(
                            stoul(param("limit")));
                    }
                    catch (...)
                    {
                    }
                }

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();

                WarehouseManager& warehouseManager =
                    warehouseSystem->getWarehouseManager();

                vector<LedgerRow> rows =
                    warehouseSystem->getMovementLogger().readAll();

                crow::json::wvalue response;

                crow::json::wvalue::list list;

                for (size_t i = rows.size(); i > 0; i--)
                {
                    const LedgerRow& row = rows[i - 1];

                    if (!material.empty() &&
                        row.materialID != material)
                    {
                        continue;
                    }

                    if (!product.empty() &&
                        row.productID != product)
                    {
                        continue;
                    }

                    if (!warehouse.empty() &&
                        row.warehouse != warehouse)
                    {
                        continue;
                    }

                    if (!type.empty() &&
                        row.type.find(type) == string::npos)
                    {
                        continue;
                    }

                    string day = row.timestamp.substr(0, 10);

                    if (!from.empty() && day < from)
                    {
                        continue;
                    }

                    if (!to.empty() && day > to)
                    {
                        continue;
                    }

                    if (list.size() >= limit)
                    {
                        break;
                    }

                    crow::json::wvalue item;

                    item["timestamp"] = row.timestamp;

                    item["user"] = row.user;

                    item["type"] = row.type;

                    item["warehouse"] = row.warehouse;

                    string warehouseName = "";

                    if (!row.warehouse.empty())
                    {
                        try
                        {
                            Warehouse* found =
                                warehouseManager.findWarehouse(
                                    stoi(row.warehouse));

                            if (found != nullptr)
                            {
                                warehouseName = found->getName();
                            }
                        }
                        catch (...)
                        {
                        }
                    }

                    item["warehouseName"] = warehouseName;

                    item["materialID"] = row.materialID;

                    string materialName = row.materialName;

                    if (materialName.empty() &&
                        !row.materialID.empty())
                    {
                        Material* found =
                            materialManager.findMaterial(
                                row.materialID);

                        if (found != nullptr)
                        {
                            materialName = found->getName();
                        }
                    }

                    item["materialName"] = materialName;

                    item["productID"] = row.productID;

                    item["productName"] = row.productName;

                    item["qtyIn"] = row.qtyIn;

                    item["qtyOut"] = row.qtyOut;

                    item["balance"] = row.balance;

                    item["reference"] = row.reference;

                    item["details"] = row.details;

                    item["comment"] = row.comment;

                    list.push_back(std::move(item));
                }

                response["movements"] = std::move(list);

                return response;
            });


    // ============================================================
    // QUOTATIONS (supplier quotations with price tiers by quantity)
    // ============================================================
    // See QuotationManager.h. The newest quotation per Supplier and
    // Material is the current one; older ones stay on file as
    // "Replaced" and are only listed when includePast=1.

    // List (filters: material, supplier, status, includePast)

    CROW_ROUTE(app, "/api/quotations")
        .methods(crow::HTTPMethod::GET)
        ([warehouseSystem](const crow::request& req)
            {
                QuotationManager& quotationManager =
                    warehouseSystem->getQuotationManager();

                string material =
                    req.url_params.get("material") ?
                    req.url_params.get("material") : "";

                string supplier =
                    req.url_params.get("supplier") ?
                    req.url_params.get("supplier") : "";

                string status =
                    req.url_params.get("status") ?
                    req.url_params.get("status") : "";

                bool includePast =
                    req.url_params.get("includePast") != nullptr &&
                    string(req.url_params.get("includePast")) == "1";

                const auto& all = quotationManager.getQuotations();

                // Newest first (issue date, then ID)

                vector<const Quotation*> sorted;

                for (const auto& q : all)
                {
                    sorted.push_back(q.get());
                }

                sort(sorted.begin(), sorted.end(),
                    [](const Quotation* a, const Quotation* b)
                    {
                        if (a->issueDate != b->issueDate)
                        {
                            return a->issueDate > b->issueDate;
                        }

                        return a->id > b->id;
                    });

                crow::json::wvalue response;

                crow::json::wvalue::list list;

                int pastHidden = 0;

                for (const Quotation* q : sorted)
                {
                    string qStatus = quotationManager.status(*q);

                    if (!supplier.empty() && q->supplierName != supplier)
                    {
                        continue;
                    }

                    if (!material.empty() &&
                        q->findLine(material) == nullptr)
                    {
                        continue;
                    }

                    if (!status.empty() && qStatus != status)
                    {
                        continue;
                    }

                    if (!includePast && qStatus == "Replaced" &&
                        status.empty())
                    {
                        pastHidden++;

                        continue;
                    }

                    list.push_back(
                        quotationToJson(warehouseSystem, *q));
                }

                response["quotations"] = std::move(list);

                response["pastHidden"] = pastHidden;

                return response;
            });

    // One quotation

    CROW_ROUTE(app, "/api/quotations/<string>")
        .methods(crow::HTTPMethod::GET)
        ([warehouseSystem](string id)
            {
                const Quotation* q =
                    warehouseSystem->getQuotationManager()
                        .findQuotation(id);

                if (q == nullptr)
                {
                    return crow::response(
                        404,
                        "Quotation not found.");
                }

                return crow::response(
                    quotationToJson(warehouseSystem, *q));
            });

    // Create

    CROW_ROUTE(app, "/api/quotations/create")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(400, "Invalid JSON.");
                    }

                    Quotation data;

                    string error =
                        quotationFromJson(body, data);

                    string newID;

                    if (error.empty())
                    {
                        warehouseSystem->getQuotationManager()
                            .create(data, error, &newID);
                    }

                    if (!error.empty())
                    {
                        return crow::response(400, error);
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["id"] = newID;

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // Update (everything except the ID and the PDF)

    CROW_ROUTE(app, "/api/quotations/update")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(400, "Invalid JSON.");
                    }

                    string id = quotationJsonString(body, "id");

                    Quotation data;

                    string error =
                        quotationFromJson(body, data);

                    if (error.empty())
                    {
                        warehouseSystem->getQuotationManager()
                            .update(id, data, error);
                    }

                    if (!error.empty())
                    {
                        return crow::response(400, error);
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // Delete (the PDF, if any, stays in the Documents Folder)

    CROW_ROUTE(app, "/api/quotations/delete")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);

                    if (!body)
                    {
                        return crow::response(400, "Invalid JSON.");
                    }

                    string error;

                    string deleteID = quotationJsonString(body, "id");

                    vector<string> linkedInvoices =
                        quotationInvoiceIDs(warehouseSystem, deleteID);

                    if (!linkedInvoices.empty())
                    {
                        string names;

                        for (const string& invoiceID : linkedInvoices)
                        {
                            names += (names.empty() ? "" : ", ") + invoiceID;
                        }

                        return crow::response(
                            400,
                            "Quotation " + deleteID + " cannot be deleted: "
                            "it is linked to invoice(s) " + names + ".");
                    }

                    if (!warehouseSystem->getQuotationManager()
                        .remove(deleteID, error))
                    {
                        return crow::response(400, error);
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // Upload the quotation PDF (a new upload replaces the previous
    // one). Saved under the Documents Folder as
    // "QUO-000001 - quotation.pdf", like the Procurement documents.

    CROW_ROUTE(app, "/api/quotations/<string>/document")
        .methods(crow::HTTPMethod::POST)
        ([warehouseSystem](const crow::request& req, string id)
            {
                try
                {
                    QuotationManager& quotationManager =
                        warehouseSystem->getQuotationManager();

                    if (quotationManager.findQuotation(id) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Quotation not found.");
                    }

                    string documentsFolder =
                        warehouseSystem->getDataManager()
                            .getDocumentsFolder();

                    if (documentsFolder.empty())
                    {
                        return crow::response(
                            400,
                            "Set a Documents Folder first in Data "
                            "Management Settings.");
                    }

                    auto body = crow::json::load(req.body);

                    if (!body || !body.has("fileData"))
                    {
                        return crow::response(
                            400,
                            "fileData is required.");
                    }

                    string fileData = body["fileData"].s();

                    size_t commaPosition = fileData.find(',');

                    if (commaPosition == string::npos)
                    {
                        return crow::response(
                            400,
                            "Invalid file data.");
                    }

                    string decoded =
                        decodeBase64(
                            fileData.substr(commaPosition + 1));

                    fs::create_directories(documentsFolder);

                    fs::path filePath =
                        fs::path(documentsFolder) /
                        (id + " - quotation.pdf");

                    ofstream outFile(filePath, ios::binary);

                    if (!outFile)
                    {
                        return crow::response(
                            500,
                            "Could not write file.");
                    }

                    outFile.write(decoded.data(), decoded.size());

                    outFile.close();

                    quotationManager.setDocumentPath(
                        id,
                        filePath.string());

                    crow::json::wvalue response;

                    response["success"] = true;

                    return crow::response(response);
                }
                catch (const exception& e)
                {
                    return crow::response(
                        500,
                        string("Error: ") + e.what());
                }
            });

    // View the quotation PDF

    CROW_ROUTE(app, "/api/quotations/<string>/document/file")
        ([warehouseSystem](string id)
            {
                const Quotation* q =
                    warehouseSystem->getQuotationManager()
                        .findQuotation(id);

                if (q == nullptr || q->documentPath.empty() ||
                    !fs::exists(q->documentPath))
                {
                    return crow::response(
                        404,
                        "No PDF on file for this quotation.");
                }

                ifstream inFile(q->documentPath, ios::binary);

                ostringstream contents;

                contents << inFile.rdbuf();

                crow::response res(contents.str());

                res.set_header("Content-Type", "application/pdf");

                return res;
            });


    // ============================================================
    // PRICE INFO OF ONE MATERIAL (quotations + estimate + timeline)
    // ============================================================
    // ?quantity=N prices the quotation tiers for that quantity,
    // ?date=YYYY-MM-DD reconstructs the cost on a past date,
    // ?lite=1 skips the timeline (used by the invoice forms).

    CROW_ROUTE(app, "/api/materials/<string>/price-info")
        ([warehouseSystem](const crow::request& req, string materialID)
            {
                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();

                Material* material =
                    materialManager.findMaterial(materialID);

                if (material == nullptr)
                {
                    return crow::response(404, "Material not found.");
                }

                double quantity = 1.0;

                string asOfDate;

                costQueryParams(req, quantity, asOfDate);

                bool hasQuantity =
                    req.url_params.get("quantity") != nullptr &&
                    *req.url_params.get("quantity") != '\0';

                if (!hasQuantity)
                {
                    quantity = 0.0;
                }

                bool lite =
                    req.url_params.get("lite") != nullptr &&
                    string(req.url_params.get("lite")) == "1";

                PurchaseManager& purchaseManager =
                    warehouseSystem->getPurchaseManager();

                QuotationManager& quotationManager =
                    warehouseSystem->getQuotationManager();

                crow::json::wvalue response;

                response["materialID"] = materialID;

                response["materialName"] = material->getName();

                response["uom"] = material->getUoM();

                response["quantity"] = quantity;

                response["asOfDate"] = asOfDate;

                response["primarySupplier"] =
                    material->getSupplier() != nullptr ?
                    material->getSupplier()->getName() : "";

                response["estimate"] =
                    estimateToJson(
                        CostEstimator::estimate(
                            materialID, quantity, asOfDate,
                            purchaseManager, quotationManager,
                            materialManager, !hasQuantity));

                // Every quotation line of the material, newest first

                crow::json::wvalue::list quoteList;

                for (const QuotationHit& hit :
                    quotationManager.findAllForMaterial(materialID))
                {
                    const Quotation& q = *hit.quotation;

                    crow::json::wvalue item;

                    item["id"] = q.id;
                    item["supplier"] = q.supplierName;
                    item["supplierQuoteNumber"] = q.supplierQuoteNumber;
                    item["issueDate"] = q.issueDate;
                    item["validUntil"] = q.validUntil;
                    item["status"] = quotationManager.status(q);
                    item["current"] =
                        quotationManager.isLineCurrent(q, materialID);
                    item["currency"] = q.currency;
                    item["exchangeRate"] = q.exchangeRate;
                    item["leadTimeWeeks"] = q.leadTimeWeeks;
                    item["incoterm"] = q.incoterm;
                    item["moq"] = hit.line->moq;
                    item["usualQuantity"] = hit.line->usualQuantity;
                    item["hasDocument"] =
                        !q.documentPath.empty() &&
                        fs::exists(q.documentPath);
                    item["primarySupplier"] =
                        material->getSupplier() != nullptr &&
                        material->getSupplier()->getName() ==
                            q.supplierName;

                    crow::json::wvalue::list tierList;

                    for (const auto& tier : hit.line->tiers)
                    {
                        crow::json::wvalue t;

                        t["minQuantity"] = tier.minQuantity;
                        t["unitPrice"] = tier.unitPrice;
                        t["unitPriceEUR"] =
                            QuotationManager::unitPriceEUR(q, tier);

                        tierList.push_back(std::move(t));
                    }

                    item["tiers"] = std::move(tierList);

                    // Price of the asked quantity on this quotation

                    bool below = false;

                    const QuotationTier& used =
                        QuotationManager::tierFor(
                            *hit.line,
                            quantity > 0.0 ? quantity :
                            QuotationManager::referenceQuantity(
                                *hit.line),
                            &below);

                    item["tierMinQuantity"] = used.minQuantity;
                    item["tierPrice"] = used.unitPrice;
                    item["tierPriceEUR"] =
                        QuotationManager::unitPriceEUR(q, used);
                    item["belowFirstTier"] = quantity > 0.0 && below;
                    item["belowMoq"] =
                        quantity > 0.0 && hit.line->moq > 0.0 &&
                        quantity < hit.line->moq;

                    quoteList.push_back(std::move(item));
                }

                response["quotations"] = std::move(quoteList);

                if (lite)
                {
                    return crow::response(response);
                }

                // Timeline: invoices (real, landed price) and
                // quotations (offered price) of this material together

                struct TimelineRow
                {
                    string date;
                    string sortKey;
                    crow::json::wvalue json;
                };

                vector<TimelineRow> rows;

                for (const auto& invoice : purchaseManager.getInvoices())
                {
                    const auto& lines = invoice->getLines();

                    for (size_t i = 0; i < lines.size(); i++)
                    {
                        if (lines[i].materialID != materialID)
                        {
                            continue;
                        }

                        double rate = invoice->getExchangeRate();

                        double importPerUnit =
                            lines[i].receivedQuantity > 0.0 ?
                            invoice->getLineAllocatedCost(i) /
                            lines[i].receivedQuantity * rate : 0.0;

                        double otherPerUnit =
                            purchaseManager.getLineOtherCostAdditionEUR(
                                invoice.get(), i);

                        TimelineRow row;

                        row.date = invoice->getDate();
                        row.sortKey = invoice->getDate() + "1" +
                            invoice->getID();

                        row.json["type"] = "Invoice";
                        row.json["date"] = invoice->getDate();
                        row.json["ref"] = invoice->getID();
                        row.json["supplier"] =
                            material->getSupplier() != nullptr ?
                            material->getSupplier()->getName() : "";
                        row.json["quantity"] = lines[i].receivedQuantity;
                        row.json["basePriceEUR"] =
                            lines[i].unitCost * rate;
                        row.json["extraPerUnitEUR"] =
                            importPerUnit + otherPerUnit;
                        row.json["unitPriceEUR"] =
                            purchaseManager.getLineLandedUnitPriceEUR(
                                invoice.get(), i);
                        row.json["quotationID"] = lines[i].quotationID;
                        row.json["quotedUnitPriceEUR"] =
                            lines[i].quotedUnitPriceEUR;

                        rows.push_back(std::move(row));
                    }
                }

                for (const QuotationHit& hit :
                    quotationManager.findAllForMaterial(materialID))
                {
                    const Quotation& q = *hit.quotation;

                    TimelineRow row;

                    row.date = q.issueDate;
                    row.sortKey = q.issueDate + "0" + q.id;

                    row.json["type"] = "Quotation";
                    row.json["date"] = q.issueDate;
                    row.json["ref"] = q.id;
                    row.json["supplier"] = q.supplierName;
                    row.json["status"] = quotationManager.status(q);
                    row.json["current"] =
                        quotationManager.isLineCurrent(q, materialID);

                    crow::json::wvalue::list tierList;

                    for (const auto& tier : hit.line->tiers)
                    {
                        crow::json::wvalue t;

                        t["minQuantity"] = tier.minQuantity;
                        t["unitPriceEUR"] =
                            QuotationManager::unitPriceEUR(q, tier);

                        tierList.push_back(std::move(t));
                    }

                    row.json["tiers"] = std::move(tierList);

                    rows.push_back(std::move(row));
                }

                sort(rows.begin(), rows.end(),
                    [](const TimelineRow& a, const TimelineRow& b)
                    {
                        return a.sortKey > b.sortKey;
                    });

                crow::json::wvalue::list timeline;

                for (auto& row : rows)
                {
                    timeline.push_back(std::move(row.json));
                }

                response["timeline"] = std::move(timeline);

                return crow::response(response);
            });

    // ============================================================
    // PRICE ESTIMATES OF SEVERAL MATERIALS AT THEIR QUANTITIES
    // ============================================================
    // ?items=ID:qty;ID:qty  (&date=YYYY-MM-DD)

    CROW_ROUTE(app, "/api/price-estimates")
        ([warehouseSystem](const crow::request& req)
            {
                double ignoredQuantity = 1.0;

                string asOfDate;

                costQueryParams(req, ignoredQuantity, asOfDate);

                const char* itemsParam = req.url_params.get("items");

                string items = itemsParam != nullptr ? itemsParam : "";

                MaterialManager& materialManager =
                    warehouseSystem->getMaterialManager();

                crow::json::wvalue::list list;

                size_t start = 0;

                while (start <= items.size())
                {
                    size_t end = items.find(';', start);

                    if (end == string::npos)
                    {
                        end = items.size();
                    }

                    string token = items.substr(start, end - start);

                    start = end + 1;

                    size_t colon = token.find(':');

                    if (token.empty() || colon == string::npos)
                    {
                        continue;
                    }

                    string materialID = token.substr(0, colon);

                    double quantity = atof(token.c_str() + colon + 1);

                    Material* material =
                        materialManager.findMaterial(materialID);

                    if (material == nullptr)
                    {
                        continue;
                    }

                    crow::json::wvalue item;

                    item["materialID"] = materialID;

                    item["materialName"] = material->getName();

                    item["quantity"] = quantity;

                    item["estimate"] =
                        estimateToJson(
                            CostEstimator::estimate(
                                materialID, quantity, asOfDate,
                                warehouseSystem->getPurchaseManager(),
                                warehouseSystem->getQuotationManager(),
                                materialManager));

                    list.push_back(std::move(item));
                }

                crow::json::wvalue response;

                response["items"] = std::move(list);

                return response;
            });

    // ============================================================
    // START SERVER
    // ============================================================

    cout << endl;

    cout << "========================================"
        << endl;

    cout << "WAREHOUSE MANAGEMENT SYSTEM"
        << endl;

    cout << "Web interface running at:"
        << endl;

    cout << "http://localhost:18080"
        << endl;

    cout << "========================================"
        << endl;


    app.port(18080)
        .multithreaded()
        .run();
}