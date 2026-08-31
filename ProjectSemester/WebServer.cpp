#include "WebServer.h"

#include <crow.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

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
// RUN
// ================================================================

void WebServer::run()
{
    crow::SimpleApp app;

    WarehouseSystem* warehouseSystem = system;

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
                            "Invalid Material ID. Expected format ###-######.");
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

                    string supplier =
                        body["supplier"].s();

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
                        supplier,
                        photoPath,
                        active);

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

                    item["supplier"] =
                        material->getSupplier();

                    item["photo"] =
                        material->getPhotoPath();

                    item["active"] =
                        material->isActive();

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

                    string supplier =
                        body["supplier"].s();

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
                            "Invalid Material ID. Expected format ###-######.");
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
                        supplier,
                        photoPath,
                        active);


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

                response["supplier"] =
                    material->getSupplier();

                response["photo"] =
                    material->getPhotoPath();

                response["active"] =
                    material->isActive();


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
                            "Invalid Material ID. Expected format ###-######.");
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


                    // ------------------------------------------------
                    // Validate
                    // ------------------------------------------------

                    if (!Material::isValidID(materialID))
                    {
                        return crow::response(
                            400,
                            "Invalid Material ID. Expected format ###-######.");
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
                            quantity);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Goods receipt failed.");
                    }


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


                    // ------------------------------------------------
                    // Validate
                    // ------------------------------------------------

                    if (!Material::isValidID(
                        materialID))
                    {
                        return crow::response(
                            400,
                            "Invalid Material ID. Expected format ###-######.");
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
                            quantity);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Goods issue failed.");
                    }


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
                            "Invalid Material ID. Expected format ###-######.");
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
                            quantity);


                    if (!success)
                    {
                        return crow::response(
                            400,
                            "Transfer failed. Check warehouses, material, and available quantity.");
                    }


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
// GET PRODUCTS
// ============================================================

    CROW_ROUTE(app, "/api/products")
        ([warehouseSystem]()
            {
                crow::json::wvalue response;

                crow::json::wvalue::list productList;


                ProductManager& productManager =
                    warehouseSystem->getProductManager();


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

                        bom["quantity"] =
                            bomItem.quantity;


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


                return response;
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