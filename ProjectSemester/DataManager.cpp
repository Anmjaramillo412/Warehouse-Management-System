#include "DataManager.h"
#include <xlnt/xlnt.hpp>
#include <iostream>
using namespace std;


// ================================================================
// CONSTRUCTOR
// ================================================================

DataManager::DataManager(string file)
{
    filename = file;
    movementLogger = nullptr;
}

// ================================================================
// SET MOVEMENT LOGGER
// ================================================================

void DataManager::setMovementLogger(
    MovementLogger* logger)
{
    movementLogger = logger;
}

// ================================================================
// SAVE
// ================================================================

bool DataManager::save(
    MaterialManager& materialManager,
    WarehouseManager& warehouseManager,
    ProductManager& productManager)
{
    try
    {
        xlnt::workbook workbook;


        // ========================================================
        // MATERIALS SHEET
        // ========================================================

        xlnt::worksheet materialsSheet =
            workbook.active_sheet();

        materialsSheet.title("Materials");


        materialsSheet.cell("A1").value("Material ID");
        materialsSheet.cell("B1").value("Name");
        materialsSheet.cell("C1").value("Description");
        materialsSheet.cell("D1").value("UoM");
        materialsSheet.cell("E1").value("Category");
        materialsSheet.cell("F1").value("Supplier");
        materialsSheet.cell("G1").value("Photo");
        materialsSheet.cell("H1").value("Active");


        int materialRow = 2;


        const vector<unique_ptr<Material>>& materials =
            materialManager.getMaterials();


        for (const auto& material : materials)
        {
            materialsSheet.cell(
                "A" + to_string(materialRow))
                .value(material->getID());

            materialsSheet.cell(
                "B" + to_string(materialRow))
                .value(material->getName());

            materialsSheet.cell(
                "C" + to_string(materialRow))
                .value(material->getDescription());

            materialsSheet.cell(
                "D" + to_string(materialRow))
                .value(material->getUoM());

            materialsSheet.cell(
                "E" + to_string(materialRow))
                .value(material->getCategory());

            materialsSheet.cell(
                "F" + to_string(materialRow))
                .value(material->getSupplier());

            materialsSheet.cell(
                "G" + to_string(materialRow))
                .value(material->getPhotoPath());

            materialsSheet.cell(
                "H" + to_string(materialRow))
                .value(
                    material->isActive()
                    ? "YES"
                    : "NO");

            materialRow++;
        }


        // ========================================================
        // WAREHOUSES SHEET
        // ========================================================

        xlnt::worksheet warehousesSheet =
            workbook.create_sheet();
        warehousesSheet.title("Warehouses");

        warehousesSheet.cell("A1")
            .value("Warehouse ID");

        warehousesSheet.cell("B1")
            .value("Warehouse Name");


        int warehouseRow = 2;


        vector<Warehouse*>& warehouses =
            warehouseManager.getWarehouses();


        for (const Warehouse* warehouse : warehouses)
        {
            warehousesSheet.cell(
                "A" + to_string(warehouseRow))
                .value(warehouse->getID());

            warehousesSheet.cell(
                "B" + to_string(warehouseRow))
                .value(warehouse->getName());

            warehouseRow++;
        }

        // ========================================================
        // PRODUCTS SHEET
        // ========================================================

        xlnt::worksheet productsSheet =
            workbook.create_sheet();

        productsSheet.title("Products");


        productsSheet.cell("A1")
            .value("Product ID");

        productsSheet.cell("B1")
            .value("Name");

        productsSheet.cell("C1")
            .value("Description");


        int productRow = 2;


        const auto& products =
            productManager.getProducts();


        for (const auto& product : products)
        {
            productsSheet.cell(
                "A" + to_string(productRow))
                .value(product->getID());

            productsSheet.cell(
                "B" + to_string(productRow))
                .value(product->getName());

            productsSheet.cell(
                "C" + to_string(productRow))
                .value(product->getDescription());

            productRow++;
        }

        // ========================================================
        // BOM SHEET
        // ========================================================

        xlnt::worksheet bomSheet =
            workbook.create_sheet();

        bomSheet.title("BOM");


        bomSheet.cell("A1")
            .value("Product ID");

        bomSheet.cell("B1")
            .value("Material ID");

        bomSheet.cell("C1")
            .value("Quantity");


        int bomRow = 2;


        for (const auto& product : products)
        {
            for (const auto& item :
                product->getBOM())
            {
                bomSheet.cell(
                    "A" + to_string(bomRow))
                    .value(product->getID());

                bomSheet.cell(
                    "B" + to_string(bomRow))
                    .value(item.materialID);

                bomSheet.cell(
                    "C" + to_string(bomRow))
                    .value(item.quantity);

                bomRow++;
            }
        }

        // ========================================================
        // INVENTORY SHEET
        // ========================================================

        xlnt::worksheet inventorySheet =
            workbook.create_sheet();
        inventorySheet.title("Inventory");

        inventorySheet.cell("A1")
            .value("Warehouse ID");

        inventorySheet.cell("B1")
            .value("Material ID");

        inventorySheet.cell("C1")
            .value("Quantity");


        int inventoryRow = 2;


        for (const Warehouse* warehouse : warehouses)
        {
            WarehouseNode* node =
                warehouse->getHead();


            while (node != nullptr)
            {
                inventorySheet.cell(
                    "A" + to_string(inventoryRow))
                    .value(warehouse->getID());

                inventorySheet.cell(
                    "B" + to_string(inventoryRow))
                    .value(
                        node->material->getID());

                inventorySheet.cell(
                    "C" + to_string(inventoryRow))
                    .value(node->quantity);


                inventoryRow++;

                node = node->next;
            }
        }


        // ========================================================
        // SAVE FILE
        // ========================================================

        workbook.save(filename);

        // ------------------------------------------------
        // Log data save
        // ------------------------------------------------

                if (movementLogger != nullptr)
                {
                    movementLogger->logSystemEvent(
                        "DATA SAVE",
                        "File: " + filename);
                }

        return true;
    }
    catch (const exception& e)
    {
        cout << endl;

        cout << "Error saving Excel file: "
            << e.what()
            << endl;

        return false;
    }
}

// ================================================================
// LOAD
// ================================================================

bool DataManager::load(
    MaterialManager& materialManager,
    WarehouseManager& warehouseManager,
    ProductManager& productManager)
{
    try
    {
        // ========================================================
        // OPEN WORKBOOK
        // ========================================================

        xlnt::workbook workbook;

        workbook.load(filename);


        // ========================================================
        // CLEAR CURRENT DATA
        // ========================================================

        materialManager.clear();
        productManager.clear();


        // Delete existing warehouses

        vector<Warehouse*>& currentWarehouses =
            warehouseManager.getWarehouses();


        for (Warehouse* warehouse : currentWarehouses)
        {
            delete warehouse;
        }

        currentWarehouses.clear();


        // ========================================================
        // LOAD MATERIALS
        // ========================================================

        if (!workbook.contains("Materials"))
        {
            cout << endl;

            cout << "Materials sheet not found."
                << endl;

            return false;
        }


        xlnt::worksheet materialsSheet =
            workbook.sheet_by_title("Materials");


        for (auto row : materialsSheet.rows(false))
        {
            if (row[0].row() == 1)
            {
                continue;
            }


            string id =
                row[0].value<string>();


            if (id.empty())
            {
                continue;
            }


            string name =
                row[1].value<string>();

            string description =
                row[2].value<string>();

            string uom =
                row[3].value<string>();

            string category =
                row[4].value<string>();

            string supplier =
                row[5].value<string>();

            string photoPath =
                row[6].value<string>();

            string activeValue =
                row[7].value<string>();


            bool active =
                (activeValue == "YES");


            Material material(
                id,
                name,
                description,
                uom,
                category,
                supplier,
                photoPath,
                active);


            if (!materialManager.createMaterial(material))
            {
                cout << endl;

                cout << "Warning: Could not load material "
                    << id
                    << endl;
            }
        }


        // ========================================================
        // LOAD WAREHOUSES
        // ========================================================

        if (!workbook.contains("Warehouses"))
        {
            cout << endl;

            cout << "Warehouses sheet not found."
                << endl;
        }
        else
        {
            xlnt::worksheet warehousesSheet =
                workbook.sheet_by_title("Warehouses");


            for (auto row :
                warehousesSheet.rows(false))
            {
                if (row[0].row() == 1)
                {
                    continue;
                }


                int warehouseID =
                    row[0].value<int>();


                string warehouseName =
                    row[1].value<string>();


                warehouseManager.createWarehouse(
                    warehouseID,
                    warehouseName);
            }
        }

        // ========================================================
        // LOAD INVENTORY
        // ========================================================

        if (!workbook.contains("Inventory"))
        {
            cout << endl;

            cout << "Inventory sheet not found."
                << endl;
        }
        else
        {
            xlnt::worksheet inventorySheet =
                workbook.sheet_by_title("Inventory");


            for (auto row :
                inventorySheet.rows(false))
            {
                if (row[0].row() == 1)
                {
                    continue;
                }


                int warehouseID =
                    row[0].value<int>();


                string materialID =
                    row[1].value<string>();


                int quantity =
                    row[2].value<int>();


                Warehouse* warehouse =
                    warehouseManager.findWarehouse(
                        warehouseID);


                Material* material =
                    materialManager.findMaterial(
                        materialID);


                if (warehouse == nullptr)
                {
                    cout << endl;

                    cout << "Warning: Warehouse "
                        << warehouseID
                        << " not found."
                        << endl;

                    continue;
                }


                if (material == nullptr)
                {
                    cout << endl;

                    cout << "Warning: Material "
                        << materialID
                        << " not found."
                        << endl;

                    continue;
                }


                if (!warehouse->addMaterial(
                    material,
                    quantity))
                {
                    cout << endl;

                    cout << "Warning: Could not load inventory "
                        << materialID
                        << " into warehouse "
                        << warehouseID
                        << endl;
                }
            }
        }

        // ========================================================
        // LOAD PRODUCTS
        // ========================================================

        if (!workbook.contains("Products"))
        {
            cout << endl;

            cout << "Products sheet not found."
                << endl;
        }
        else
        {
            xlnt::worksheet productsSheet =
                workbook.sheet_by_title("Products");


            for (auto row :
                productsSheet.rows(false))
            {
                if (row[0].row() == 1)
                {
                    continue;
                }


                string id =
                    row[0].value<string>();


                if (id.empty())
                {
                    continue;
                }


                string name =
                    row[1].value<string>();


                string description =
                    row[2].value<string>();


                Product product(
                    id,
                    name,
                    description);


                if (!productManager.createProduct(
                    product))
                {
                    cout << endl;

                    cout << "Warning: Could not load product "
                        << id
                        << endl;
                }
            }
        }

        // ========================================================
        // LOAD BOM
        // ========================================================

        if (!workbook.contains("BOM"))
        {
            cout << endl;

            cout << "BOM sheet not found."
                << endl;
        }
        else
        {
            xlnt::worksheet bomSheet =
                workbook.sheet_by_title("BOM");


            for (auto row :
                bomSheet.rows(false))
            {
                if (row[0].row() == 1)
                {
                    continue;
                }


                string productID =
                    row[0].value<string>();


                string materialID =
                    row[1].value<string>();


                int quantity =
                    row[2].value<int>();


                Product* product =
                    productManager.findProduct(
                        productID);


                Material* material =
                    materialManager.findMaterial(
                        materialID);


                if (product == nullptr)
                {
                    cout << endl;

                    cout << "Warning: Product "
                        << productID
                        << " not found."
                        << endl;

                    continue;
                }


                if (material == nullptr)
                {
                    cout << endl;

                    cout << "Warning: Material "
                        << materialID
                        << " not found for product "
                        << productID
                        << endl;

                    continue;
                }


                if (!product->addBOMItem(
                    materialID,
                    quantity))
                {
                    cout << endl;

                    cout << "Warning: Could not load BOM item."
                        << endl;
                }
            }
        }

        // ========================================================
        // LOAD COMPLETED
        // ========================================================

        if (movementLogger != nullptr)
        {
            movementLogger->logSystemEvent(
                "DATA LOAD",
                "File: " + filename);
        }

        return true;
    }
    catch (const exception& e)
    {
        cout << endl;

        cout << "Error loading Excel file: "
            << e.what()
            << endl;

        return false;
    }
}