#include "DataManager.h"
#include <xlnt/xlnt.hpp>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <sstream>
using namespace std;


// ================================================================
// FIELD SANITIZING (for the packed "Additional Suppliers" cell)
// ================================================================
// That one cell packs several Supplier entries as
// "Name,PartNumber~Name,PartNumber" - a comma or "~" inside a
// Supplier name or Part Number would be misread as a separator, so
// strip them here the same way ProcurementManager sanitizes its own
// packed fields.

namespace
{
    string sanitizePackedField(const string& value)
    {
        string result = value;

        replace(result.begin(), result.end(), ',', ';');
        replace(result.begin(), result.end(), '~', ' ');

        return result;
    }
}


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
    SupplierManager& supplierManager,
    WarehouseManager& warehouseManager,
    ProductManager& productManager)
{
    try
    {
        // ------------------------------------------------
        // Make sure the target directory exists
        // (e.g. "data/") before xlnt tries to write to it
        // ------------------------------------------------

        std::filesystem::path targetPath(filename);

        if (targetPath.has_parent_path())
        {
            std::filesystem::create_directories(
                targetPath.parent_path());
        }

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
        materialsSheet.cell("F1").value("Type");
        materialsSheet.cell("G1").value("Drawing Number");
        materialsSheet.cell("H1").value("Manufacturer");
        materialsSheet.cell("I1").value("Manufacturer Part Number");
        materialsSheet.cell("J1").value("Supplier");
        materialsSheet.cell("K1").value("Supplier Part Number");
        materialsSheet.cell("L1").value("Photo");
        materialsSheet.cell("M1").value("Active");
        materialsSheet.cell("N1").value("Additional Suppliers");


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
                .value(
                    Material::materialTypeToString(
                        material->getType()));

            materialsSheet.cell(
                "G" + to_string(materialRow))
                .value(material->getDrawingNumber());

            materialsSheet.cell(
                "H" + to_string(materialRow))
                .value(material->getManufacturer());

            materialsSheet.cell(
                "I" + to_string(materialRow))
                .value(material->getManufacturerPartNumber());

            materialsSheet.cell(
                "J" + to_string(materialRow))
                .value(
                    material->getSupplier() != nullptr
                    ? material->getSupplier()->getName()
                    : "");

            materialsSheet.cell(
                "K" + to_string(materialRow))
                .value(material->getSupplierPartNumber());

            materialsSheet.cell(
                "L" + to_string(materialRow))
                .value(material->getPhotoPath());

            materialsSheet.cell(
                "M" + to_string(materialRow))
                .value(
                    material->isActive()
                    ? "YES"
                    : "NO");

            // Additional (non-primary) Suppliers, each as
            // "Name,Supplier Part Number", joined by "~" - same
            // convention as Procurement's receipts field.
            string additionalSuppliersField = "";

            const auto& additionalSuppliers =
                material->getAdditionalSuppliers();

            for (size_t i = 0; i < additionalSuppliers.size(); i++)
            {
                if (i > 0)
                {
                    additionalSuppliersField += "~";
                }

                additionalSuppliersField +=
                    sanitizePackedField(
                        additionalSuppliers[i].supplier != nullptr
                        ? additionalSuppliers[i].supplier->getName()
                        : "") +
                    "," +
                    sanitizePackedField(
                        additionalSuppliers[i].supplierPartNumber);
            }

            materialsSheet.cell(
                "N" + to_string(materialRow))
                .value(additionalSuppliersField);

            materialRow++;
        }


        // ========================================================
        // SUPPLIERS SHEET
        // ========================================================

        xlnt::worksheet suppliersSheet =
            workbook.create_sheet();

        suppliersSheet.title("Suppliers");

        suppliersSheet.cell("A1").value("Name");
        suppliersSheet.cell("B1").value("Address");
        suppliersSheet.cell("C1").value("Country");
        suppliersSheet.cell("D1").value("Contact Name");
        suppliersSheet.cell("E1").value("Contact Email");
        suppliersSheet.cell("F1").value("Website");
        suppliersSheet.cell("G1").value("Ordering Methods");
        suppliersSheet.cell("H1").value("Payment Method");
        suppliersSheet.cell("I1").value("Lead Time (weeks)");


        int supplierRow = 2;


        const vector<unique_ptr<Supplier>>& suppliers =
            supplierManager.getSuppliers();


        for (const auto& supplier : suppliers)
        {
            suppliersSheet.cell(
                "A" + to_string(supplierRow))
                .value(supplier->getName());

            suppliersSheet.cell(
                "B" + to_string(supplierRow))
                .value(supplier->getAddress());

            suppliersSheet.cell(
                "C" + to_string(supplierRow))
                .value(supplier->getCountry());

            suppliersSheet.cell(
                "D" + to_string(supplierRow))
                .value(supplier->getContactName());

            suppliersSheet.cell(
                "E" + to_string(supplierRow))
                .value(supplier->getContactEmail());

            suppliersSheet.cell(
                "F" + to_string(supplierRow))
                .value(supplier->getWebsite());

            suppliersSheet.cell(
                "G" + to_string(supplierRow))
                .value(
                    Supplier::orderingMethodsToString(
                        supplier->getOrderingMethods()));

            suppliersSheet.cell(
                "H" + to_string(supplierRow))
                .value(supplier->getPaymentMethod());

            suppliersSheet.cell(
                "I" + to_string(supplierRow))
                .value(supplier->getLeadTimeWeeks());

            supplierRow++;
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
    SupplierManager& supplierManager,
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
        supplierManager.clear();
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
        // LOAD SUPPLIERS
        // (must happen before Materials, which reference them)
        // ========================================================

        if (!workbook.contains("Suppliers"))
        {
            cout << endl;

            cout << "Suppliers sheet not found."
                << endl;
        }
        else
        {
            xlnt::worksheet suppliersSheet =
                workbook.sheet_by_title("Suppliers");


            for (auto row : suppliersSheet.rows(false))
            {
                if (row[0].row() == 1)
                {
                    continue;
                }


                string name =
                    row[0].value<string>();


                if (name.empty())
                {
                    continue;
                }


                string address =
                    row[1].value<string>();

                string country =
                    row[2].value<string>();

                string contactName =
                    row[3].value<string>();

                string contactEmail =
                    row[4].value<string>();

                string website =
                    row[5].value<string>();

                string orderingMethodsValue =
                    row[6].value<string>();

                string paymentMethod =
                    row[7].value<string>();

                int leadTimeWeeks =
                    row[8].value<int>();


                Supplier supplier(
                    name,
                    address,
                    country,
                    contactName,
                    contactEmail,
                    website,
                    Supplier::orderingMethodsFromString(
                        orderingMethodsValue),
                    paymentMethod,
                    leadTimeWeeks);


                if (!supplierManager.createSupplier(supplier))
                {
                    cout << endl;

                    cout << "Warning: Could not load supplier "
                        << name
                        << endl;
                }
            }
        }


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

            string typeValue =
                row[5].value<string>();

            string drawingNumber =
                row[6].value<string>();

            string manufacturer =
                row[7].value<string>();

            string manufacturerPartNumber =
                row[8].value<string>();

            string supplierName =
                row[9].value<string>();

            string supplierPartNumber =
                row[10].value<string>();

            string photoPath =
                row[11].value<string>();

            string activeValue =
                row[12].value<string>();


            bool active =
                (activeValue == "YES");


            // "Additional Suppliers" (column N) is a newer column -
            // an older file simply has nothing written in that cell,
            // which xlnt reads back as an empty string (same as any
            // other column here, none of which are length-guarded).
            string additionalSuppliersField =
                row[13].value<string>();


            Supplier* supplier =
                supplierName.empty()
                ? nullptr
                : supplierManager.findSupplier(supplierName);


            if (!supplierName.empty() && supplier == nullptr)
            {
                cout << endl;

                cout << "Warning: Supplier "
                    << supplierName
                    << " not found for material "
                    << id
                    << endl;
            }


            vector<MaterialSupplierLink> additionalSuppliers;

            if (!additionalSuppliersField.empty())
            {
                stringstream additionalStream(
                    additionalSuppliersField);

                string entry;

                while (getline(additionalStream, entry, '~'))
                {
                    if (entry.empty())
                    {
                        continue;
                    }

                    size_t commaPosition =
                        entry.find(',');

                    string additionalSupplierName =
                        commaPosition == string::npos ?
                        entry : entry.substr(0, commaPosition);

                    string additionalPartNumber =
                        commaPosition == string::npos ?
                        "" : entry.substr(commaPosition + 1);

                    Supplier* additionalSupplier =
                        supplierManager.findSupplier(
                            additionalSupplierName);

                    if (additionalSupplier == nullptr)
                    {
                        cout << endl;

                        cout << "Warning: Additional Supplier "
                            << additionalSupplierName
                            << " not found for material "
                            << id
                            << endl;

                        continue;
                    }

                    MaterialSupplierLink link;

                    link.supplier = additionalSupplier;
                    link.supplierPartNumber = additionalPartNumber;

                    additionalSuppliers.push_back(link);
                }
            }


            Material material(
                id,
                name,
                description,
                uom,
                category,
                Material::materialTypeFromString(typeValue),
                drawingNumber,
                manufacturer,
                manufacturerPartNumber,
                supplier,
                supplierPartNumber,
                photoPath,
                active);

            material.setAdditionalSuppliers(
                additionalSuppliers);


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