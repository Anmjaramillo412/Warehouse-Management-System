#include "WebServer.h"

#include <crow.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <ctime>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

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
// ORDERED COVERAGE (how much of a Projection's need is actually
// covered by real Procurement Orders)
// ================================================================
// A Procurement Order can be registered two ways: tied to one
// specific Projection (projectionID set, from that Projection's
// "Register Order" button), or untied/general (projectionID empty,
// e.g. registered straight from Procurement Orders). Summing every
// order's quantity globally for a material - regardless of which
// Projection it was tied to - let the SAME order satisfy several
// different Projections' needs for that material at once, so
// everything read "Fully Ordered" even when only one of them had
// actually been covered.
//
// By decision, an untied order does NOT count as coverage for any
// Projection - it sits in Procurement as unassigned stock-on-order
// until someone links it (or it is received, at which point it
// becomes real stock and shows up through getVirtualStock instead).
// Only an order tied directly to a Projection counts toward that
// Projection's "Fully Ordered" status.

static int sumDirectOrdered(
    const string& materialID,
    const string& projectionID,
    ProcurementManager& procurementManager)
{
    int total = 0;

    for (const auto& order : procurementManager.getOrders())
    {
        if (order->getMaterialID() == materialID &&
            order->getProjectionID() == projectionID)
        {
            total += order->getOrderedQuantity();
        }
    }

    return total;
}

static int computeOrderedCoverage(
    const string& materialID,
    int requiredQuantity,
    const string& projectionID,
    ProjectionManager& projectionManager,
    ProcurementManager& procurementManager)
{
    // requiredQuantity, projectionManager are kept as parameters so
    // every call site stays unchanged even though only the directly
    // tied orders count now - see the comment block above.

    (void)requiredQuantity;
    (void)projectionManager;

    return sumDirectOrdered(
        materialID,
        projectionID,
        procurementManager);
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
    crow::SimpleApp app;

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
                    // material is identified by its technical drawing
                    // (Drawing Number + Drawing Version) and has no
                    // Manufacturer, Manufacturer Part Number, Supplier
                    // or Supplier Part Number at all; a Standard Part
                    // is the reverse - see Material::
                    // requiresDrawingNumber().
                    // ------------------------------------------------

                    SupplierManager& supplierManager =
                        warehouseSystem->getSupplierManager();

                    Supplier* supplier = nullptr;

                    vector<MaterialSupplierLink> additionalSuppliers;

                    if (Material::requiresDrawingNumber(type))
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
                        // Supplier at all.
                        manufacturer = "";
                        manufacturerPartNumber = "";
                        supplierPartNumber = "";
                    }
                    else
                    {
                        // Standard Part: no drawing at all. The
                        // Supplier itself is optional here - a
                        // material can be registered before a
                        // supplier has been sourced/confirmed. If a
                        // name WAS given, though, it must resolve to
                        // a real Supplier (an unresolved name is
                        // treated as a typo, not "no supplier").
                        drawingNumber = "";
                        drawingVersion = "";

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

                            // --------------------------------------
                            // Resolve additional (non-primary)
                            // Suppliers - only meaningful once
                            // there is a primary Supplier.
                            // --------------------------------------

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
                            // No Supplier yet - Supplier Part
                            // Number and Additional Suppliers don't
                            // apply without a primary Supplier.
                            supplierPartNumber = "";
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

                    if (Material::requiresDrawingNumber(type))
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

                        manufacturer = "";
                        manufacturerPartNumber = "";
                        supplierPartNumber = "";
                    }
                    else
                    {
                        // Supplier is optional here too - see the
                        // same block in /api/materials/create.
                        drawingNumber = "";
                        drawingVersion = "";

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
                        leadTimeWeeks);

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
                        leadTimeWeeks);

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
                    return crow::response(
                        500,
                        "Error saving data.");
                }


                return crow::response(
                    200,
                    "Data saved successfully.");
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

                    int quantity =
                        body["quantity"].i();

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

                    int quantity =
                        body["quantity"].i();

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

                    if (node->quantity < quantity)
                    {
                        return crow::response(
                            409,
                            "Insufficient quantity available.");
                    }


                    // ------------------------------------------------
                    // Execute Goods Issue
                    // ------------------------------------------------

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

                    int quantity =
                        body["quantity"].i();

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


                    // ------------------------------------------------
                    // Create Product
                    // ------------------------------------------------

                    Product product(
                        id,
                        name,
                        description);


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

                            int quantity =
                                item["quantity"].i();


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

                    if (productManager.findProduct(id) == nullptr)
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

                            int quantity =
                                item["quantity"].i();


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
                            bomItem.quantity = quantity;

                            bom.push_back(bomItem);
                        }
                    }


                    if (!productManager.modifyProduct(
                        id,
                        name,
                        description,
                        bom))
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

                        bom["quantity"] =
                            bomItem.quantity;


                        // ------------------------------------------------
                        // Stock of this material in each Warehouse, so the
                        // UI can flag whether there is enough on hand to
                        // build 10 units of this Product (10 * bomItem's
                        // per-unit quantity, per Warehouse).
                        // ------------------------------------------------

                        crow::json::wvalue::list warehouseStockList;


                        for (const Warehouse* warehouse : warehouses)
                        {
                            crow::json::wvalue stockItem;

                            stockItem["warehouseID"] =
                                warehouse->getID();

                            stockItem["warehouseName"] =
                                warehouse->getName();


                            WarehouseNode* node =
                                warehouse->findMaterial(
                                    bomItem.materialID);

                            int quantity =
                                (node != nullptr) ?
                                node->quantity : 0;

                            stockItem["quantity"] =
                                quantity;

                            stockItem["sufficientForSafetyStock"] =
                                quantity >= (bomItem.quantity *
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

                    int quantity =
                        body["quantity"].i();


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

                    ProductManager& productManager =
                        warehouseSystem->getProductManager();


                    InventoryManager& inventoryManager =
                        warehouseSystem->getInventoryManager();


                    bool success =
                        inventoryManager.sellProduct(
                            productManager,
                            warehouseID,
                            productID,
                            quantity);


                    if (!success)
                    {
                        return crow::response(
                            409,
                            "Sale failed. Check product, warehouse, and component stock.");
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

                    item["totalReceivedQuantity"] =
                        order->getTotalReceivedQuantity();

                    item["pendingQuantity"] =
                        order->getPendingQuantity();

                    item["status"] =
                        order->getStatus();


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
// PROJECTIONS - LIST
// ============================================================

    CROW_ROUTE(app, "/api/projections")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list projectionList;


                ProjectionManager& projectionManager =
                    warehouseSystem->getProjectionManager();

                ProcurementManager& procurementManager =
                    warehouseSystem->getProcurementManager();

                ProductManager& productManager =
                    warehouseSystem->getProductManager();


                for (const auto& projection :
                    projectionManager.getProjections())
                {
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


                    // Status: "Fully Ordered" once every item's
                    // required quantity is actually covered - see
                    // computeOrderedCoverage() above for why a plain
                    // global sum over-credited shared orders to more
                    // than one Projection at once.

                    bool fullyOrdered = true;

                    for (const auto& projItem :
                        projection->getItems())
                    {
                        int ordered =
                            computeOrderedCoverage(
                                projItem.materialID,
                                projItem.requiredQuantity,
                                projection->getID(),
                                projectionManager,
                                procurementManager);

                        if (ordered < projItem.requiredQuantity)
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
                    // Coverage from real Procurement Orders - orders
                    // tied to this Projection count for it directly;
                    // untied orders are shared with every other still
                    // open Projection that also needs this material,
                    // so the same units are never credited twice (see
                    // computeOrderedCoverage() above).

                    int ordered =
                        computeOrderedCoverage(
                            projItem.materialID,
                            projItem.requiredQuantity,
                            projection->getID(),
                            projectionManager,
                            procurementManager);

                    int pending =
                        projItem.requiredQuantity - ordered;

                    if (pending < 0)
                    {
                        pending = 0;
                    }

                    if (pending > 0)
                    {
                        fullyOrdered = false;
                    }


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


                    itemJson["requiredQuantity"] =
                        projItem.requiredQuantity;

                    itemJson["stockAtCreation"] =
                        projItem.stockAtCreation;


                    // Virtual stock (live, not the snapshot taken
                    // when the projection was created): total stock
                    // minus what every other still-active Projection
                    // has already reserved for this material - see
                    // ProjectionManager::getVirtualStock(). This
                    // Projection's own reservation is excluded so it
                    // is not subtracted from itself.

                    itemJson["currentStock"] =
                        projectionManager.getVirtualStock(
                            projItem.materialID,
                            projection->getID());


                    itemJson["orderedQuantity"] =
                        ordered;

                    itemJson["pendingQuantity"] =
                        pending;


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

                    bool success =
                        projectionManager.completeProjection(
                            id,
                            producedQuantity,
                            completionDate);

                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Could not confirm production. Check that every BOM material has enough stock for the produced quantity.");
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
                            "This Projection has already been completed and archived - it cannot be deleted.");
                    }

                    bool success =
                        projectionManager.deleteProjection(id);

                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Could not delete this Projection.");
                    }

                    crow::json::wvalue response;

                    response["success"] = true;

                    response["message"] =
                        "Projection deleted successfully.";

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

                    int warehouseID =
                        body["warehouseID"].i();

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


                    WarehouseManager& warehouseManager =
                        warehouseSystem->getWarehouseManager();

                    if (warehouseManager.findWarehouse(warehouseID) == nullptr)
                    {
                        return crow::response(
                            404,
                            "Warehouse not found.");
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

                    Projection* projection =
                        projectionManager.createProjection(
                            productID,
                            warehouseID,
                            deadline,
                            manufactureQuantity,
                            string(dateBuffer));


                    if (projection == nullptr)
                    {
                        return crow::response(
                            400,
                            "Nothing to order: enough stock for all materials, "
                            "or the Product has no Bill of Materials.");
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

                    int orderedQuantity =
                        body["orderedQuantity"].i();

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
                        int orderedQuantity;
                    };

                    vector<BatchLine> lines;

                    for (const auto& lineJson : body["lines"])
                    {
                        string materialID =
                            lineJson["materialID"].s();

                        int orderedQuantity =
                            lineJson["orderedQuantity"].i();

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

                        if (orderedQuantity <= 0)
                        {
                            return crow::response(
                                400,
                                "Ordered Quantity must be greater than "
                                "zero for material " + materialID);
                        }

                        lines.push_back({materialID, orderedQuantity});
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

                    string batchID =
                        procurementManager.reserveNextID();

                    for (const auto& line : lines)
                    {
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


                    crow::json::wvalue response;

                    response["success"] = true;

                    response["id"] = batchID;

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

                    int confirmedQuantity =
                        body["confirmedQuantity"].i();


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

                    int receivedQuantity =
                        body["receivedQuantity"].i();

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

                        lineList.push_back(
                            std::move(lineItem));
                    }

                    item["lines"] =
                        std::move(lineList);


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
                            lineJson["receivedQuantity"].i();

                        line.unitCost =
                            lineJson["unitCost"].d();

                        lines.push_back(line);
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
                            lineJson["receivedQuantity"].i();

                        line.unitCost =
                            lineJson["unitCost"].d();

                        lines.push_back(line);
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
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list productList;


                PurchaseManager& purchaseManager =
                    warehouseSystem->getPurchaseManager();

                ProductManager& productManager =
                    warehouseSystem->getProductManager();


                for (const auto& product :
                    productManager.getProducts())
                {
                    ProductCostResult cost =
                        purchaseManager.computeProductCost(
                            product->getID(),
                            product->getBOM());

                    crow::json::wvalue item;

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
                        lineItem["hasPrice"] = line.hasPrice;
                        lineItem["unitPriceEUR"] = line.unitPriceEUR;
                        lineItem["lineCostEUR"] = line.lineCostEUR;

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
        (string productID)
            {
                PurchaseManager& purchaseManager =
                    warehouseSystem->getPurchaseManager();

                ProductManager& productManager =
                    warehouseSystem->getProductManager();


                Product* product =
                    productManager.findProduct(productID);

                if (product == nullptr)
                {
                    return crow::response(
                        404,
                        "Product not found.");
                }


                ProductCostResult cost =
                    purchaseManager.computeProductCost(
                        product->getID(),
                        product->getBOM());

                crow::json::wvalue response;

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
                    lineItem["hasPrice"] = line.hasPrice;
                    lineItem["unitPriceEUR"] = line.unitPriceEUR;
                    lineItem["lineCostEUR"] = line.lineCostEUR;

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