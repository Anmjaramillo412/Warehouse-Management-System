#include "ConsoleUI.h"

#include <iostream>
#include <string>
#include <limits>

using namespace std;


// ================================================================
// CONSTRUCTOR
// ================================================================

ConsoleUI::ConsoleUI()
{
}


// ================================================================
// RUN
// ================================================================

void ConsoleUI::run()
{
    mainMenu();
}


// ================================================================
// *** MAIN MENU ***
// ================================================================

void ConsoleUI::mainMenu()
{
    int option;

    bool running = true;

    while (running)
    {
        cout << endl;
        cout << "========================================" << endl;
        cout << "     WAREHOUSE MANAGEMENT SYSTEM" << endl;
        cout << "========================================" << endl;

        cout << "1. Material Manager" << endl;
        cout << "2. Warehouse Manager" << endl;
        cout << "3. Inventory Manager" << endl;
        cout << "4. Save Test" << endl;
        cout << "5. Exit" << endl;

        cout << endl;
        cout << "Select an option: ";

        cin >> option;

        switch (option)
        {
        case 1:

            materialMenu();

            break;

        case 2:

            warehouseMenu();

            break;

        case 3:

            inventoryMenu();

            break;

        case 4:

            saveData();

            break;

        case 5: 

            cout << endl;
            cout << "Exiting program..." << endl;

            running = false;

            break;

        default:

            cout << endl;
            cout << "Invalid option." << endl;

            break;
        }
    }
}

// ================================================================
// *** MATERIAL MENU ***
// ================================================================

void ConsoleUI::materialMenu()
{
    int option;

    bool running = true;

    while (running)
    {
        cout << endl;
        cout << "========================================" << endl;
        cout << "          MATERIAL MANAGER" << endl;
        cout << "========================================" << endl;

        cout << "1. Create Material" << endl;
        cout << "2. Modify Material" << endl;
        cout << "3. Delete Material" << endl;
        cout << "4. Display Materials" << endl;
        cout << "5. Search Material" << endl;
        cout << "6. Back" << endl;

        cout << endl;
        cout << "Select an option: ";

        cin >> option;

        switch (option)
        {
        case 1:

            createMaterial();

            break;

        case 2:

            modifyMaterial();

            break;

        case 3:

            deleteMaterial();

            break;

        case 4:

            displayMaterials();

            break;

        case 5:

            searchMaterial();

            break;

        case 6:

            running = false;

            break;

        default:

            cout << endl;
            cout << "Invalid option." << endl;

            break;
        }
    }
}

// ================================================================
// READ MATERIAL
// ================================================================

Material ConsoleUI::readMaterial()
{
    string id;
    string name;
    string description;
    string uom;
    string category;
    string supplier;
    string photoPath;
    bool active;


    // ID

    cout << endl;
    cout << "Material ID [###-######]: ";

    cin >> id;

    cin.ignore(
        numeric_limits<streamsize>::max(),
        '\n'
    );


    // Name

    cout << "Material Name: ";

    getline(cin, name);


    // Description

    cout << "Description: ";

    getline(cin, description);


    // Unit of Measure

    cout << "Unit of Measure [pcs]: ";

    getline(cin, uom);

    if (uom.empty())
    {
        uom = "pcs";
    }


    // Category

    cout << "Category: ";

    getline(cin, category);


    // Supplier

    cout << "Supplier: ";

    getline(cin, supplier);


    // Photo

    cout << "Photo Path: ";

    getline(cin, photoPath);


    // Active

    active = true;


    // Create Material object

    Material material(
        id,
        name,
        description,
        uom,
        category,
        supplier,
        photoPath,
        active
    );


    return material;
}

// ================================================================
// CREATE MATERIAL
// ================================================================

void ConsoleUI::createMaterial()
{
    cout << endl;
    cout << "========== CREATE MATERIAL =========="
        << endl;

    Material material = readMaterial();


    bool created =
        system.getMaterialManager()
        .createMaterial(material);


    if (created)
    {
        cout << endl;

        cout << "Material created successfully!"
            << endl;
    }
    else
    {
        cout << endl;

        cout << "ERROR: Invalid Material ID "
            << "or ID already exists!"
            << endl;
    }
}
void ConsoleUI::modifyMaterial()
{
    string id;

    cout << endl;
    cout << "========== MODIFY MATERIAL =========="
        << endl;

    cout << "Enter Material ID [###-######]: ";

    cin >> id;


    // Find material

    Material* material =
        system.getMaterialManager()
        .findMaterial(id);


    if (material == nullptr)
    {
        cout << endl;

        cout << "Material not found."
            << endl;

        return;
    }


    cin.ignore(
        numeric_limits<streamsize>::max(),
        '\n'
    );


    string name;
    string description;
    string uom;
    string category;
    string supplier;
    string photoPath;

    bool active;


    // ============================================================
    // NAME
    // ============================================================

    cout << endl;

    cout << "Current Name: "
        << material->getName()
        << endl;

    cout << "New Name: ";

    getline(cin, name);

    if (name.empty())
    {
        name = material->getName();
    }


    // ============================================================
    // DESCRIPTION
    // ============================================================

    cout << endl;

    cout << "Current Description: "
        << material->getDescription()
        << endl;

    cout << "New Description: ";

    getline(cin, description);

    if (description.empty())
    {
        description = material->getDescription();
    }


    // ============================================================
    // UOM
    // ============================================================

    cout << endl;

    cout << "Current UoM: "
        << material->getUoM()
        << endl;

    cout << "New UoM: ";

    getline(cin, uom);

    if (uom.empty())
    {
        uom = material->getUoM();
    }


    // ============================================================
    // CATEGORY
    // ============================================================

    cout << endl;

    cout << "Current Category: "
        << material->getCategory()
        << endl;

    cout << "New Category: ";

    getline(cin, category);

    if (category.empty())
    {
        category = material->getCategory();
    }


    // ============================================================
    // SUPPLIER
    // ============================================================

    cout << endl;

    cout << "Current Supplier: "
        << material->getSupplier()
        << endl;

    cout << "New Supplier: ";

    getline(cin, supplier);

    if (supplier.empty())
    {
        supplier = material->getSupplier();
    }


    // ============================================================
    // PHOTO
    // ============================================================

    cout << endl;

    cout << "Current Photo Path: "
        << material->getPhotoPath()
        << endl;

    cout << "New Photo Path: ";

    getline(cin, photoPath);

    if (photoPath.empty())
    {
        photoPath = material->getPhotoPath();
    }


    // ============================================================
    // ACTIVE
    // ============================================================

    active = material->isActive();

    char activeInput;

    cout << endl;

    cout << "Current Active Status: "
        << (active ? "Yes" : "No")
        << endl;

    cout << "Change Active Status? (y/n): ";

    cin >> activeInput;


    if (activeInput == 'y' ||
        activeInput == 'Y')
    {
        active = !active;
    }


    // ============================================================
    // CREATE UPDATED MATERIAL
    // ============================================================

    Material updatedMaterial(
        id,
        name,
        description,
        uom,
        category,
        supplier,
        photoPath,
        active
    );


    // ============================================================
    // MODIFY
    // ============================================================

    bool modified =
        system.getMaterialManager()
        .modifyMaterial(
            id,
            updatedMaterial);


    if (modified)
    {
        cout << endl;

        cout << "Material successfully modified."
            << endl;
    }
    else
    {
        cout << endl;

        cout << "Material could not be modified."
            << endl;
    }
}

// ================================================================
// DELETE MATERIAL
// ================================================================

void ConsoleUI::deleteMaterial()
{
    string id;

    cout << endl;
    cout << "========== DELETE MATERIAL =========="
        << endl;

    cout << "Enter Material ID: ";

    cin >> id;


    Material* material =
        system.getMaterialManager()
        .findMaterial(id);


    if (material == nullptr)
    {
        cout << endl;
        cout << "Material not found." << endl;

        return;
    }


    cout << endl;
    cout << "Material found:" << endl;

    material->display();


    char confirmation;

    cout << endl;
    cout << "Delete this material? (y/n): ";

    cin >> confirmation;


    if (confirmation == 'y' ||
        confirmation == 'Y')
    {
        bool deleted =
            system.getMaterialManager()
            .deleteMaterial(id);


        if (deleted)
        {
            cout << endl;
            cout << "Material deleted successfully."
                << endl;
        }
    }
    else
    {
        cout << endl;
        cout << "Delete operation cancelled."
            << endl;
    }
}


// ================================================================
// SEARCH MATERIAL
// ================================================================

void ConsoleUI::searchMaterial()
{
    string id;

    cout << endl;
    cout << "========== SEARCH MATERIAL =========="
        << endl;

    cout << "Enter Material ID [###-######]: ";

    cin >> id;

    Material* material =
        system.getMaterialManager()
        .findMaterial(id);

    if (material != nullptr)
    {
        cout << endl;
        cout << "Material found:" << endl;

        material->display();
    }
    else
    {
        cout << endl;
        cout << "Material not found." << endl;
    }
}

// ================================================================
// DISPLAY MATERIALS
// ================================================================

void ConsoleUI::displayMaterials()
{
    cout << endl;
    cout << "========== DISPLAY MATERIALS =========="
        << endl;

    system.getMaterialManager()
        .displayMaterials();
}

// ================================================================
// *** WAREHOUSE MENU ***
// ================================================================

void ConsoleUI::warehouseMenu()
{
    int option;

    bool running = true;

    while (running)
    {
        cout << endl;
        cout << "========================================" << endl;
        cout << "          WAREHOUSE MANAGER" << endl;
        cout << "========================================" << endl;

        cout << "1. Create Warehouse" << endl;
        cout << "2. Delete Warehouse" << endl;
        cout << "3. Display Warehouses" << endl;
        cout << "4. Search Warehouse" << endl;
        cout << "5. Back" << endl;

        cout << endl;
        cout << "Select an option: ";

        cin >> option;

        switch (option)
        {
        case 1:

            createWarehouse();

            break;

        case 2:

            deleteWarehouse();

            break;

        case 3:

            displayWarehouses();

            break;

        case 4:

            searchWarehouse();

            break;

        case 5:

            running = false;

            break;

        default:

            cout << endl;
            cout << "Invalid option." << endl;

            break;
        }
    }
}


// ================================================================
// CREATE WAREHOUSE
// ================================================================

void ConsoleUI::createWarehouse()
{
    int id;

    string name;


    cout << endl;
    cout << "========== CREATE WAREHOUSE =========="
        << endl;


    cout << "Warehouse ID: ";

    cin >> id;


    cin.ignore(
        numeric_limits<streamsize>::max(),
        '\n'
    );


    cout << "Warehouse Name: ";

    getline(cin, name);


    bool created =
        system.getWarehouseManager()
        .createWarehouse(id, name);


    if (created)
    {
        cout << endl;
        cout << "Warehouse created successfully!"
            << endl;
    }
    else
    {
        cout << endl;
        cout << "ERROR: Warehouse ID already exists!"
            << endl;
    }
}

// ================================================================
// SEARCH WAREHOUSE
// ================================================================

void ConsoleUI::searchWarehouse()
{
    int id;

    cout << endl;
    cout << "========== SEARCH WAREHOUSE =========="
        << endl;

    cout << "Enter Warehouse ID: ";

    cin >> id;


    Warehouse* warehouse =
        system.getWarehouseManager()
        .findWarehouse(id);


    if (warehouse != nullptr)
    {
        cout << endl;

        cout << "Warehouse found:"
            << endl;

        cout << "ID: "
            << warehouse->getID()
            << endl;

        cout << "Name: "
            << warehouse->getName()
            << endl;
    }
    else
    {
        cout << endl;

        cout << "Warehouse not found."
            << endl;
    }
}

// ================================================================
// DELETE WAREHOUSE
// ================================================================

void ConsoleUI::deleteWarehouse()
{
    int id;

    cout << endl;
    cout << "========== DELETE WAREHOUSE =========="
        << endl;

    cout << "Enter Warehouse ID: ";

    cin >> id;


    Warehouse* warehouse =
        system.getWarehouseManager()
        .findWarehouse(id);


    if (warehouse == nullptr)
    {
        cout << endl;

        cout << "Warehouse not found."
            << endl;

        return;
    }


    cout << endl;

    cout << "Warehouse found:"
        << endl;

    cout << "ID: "
        << warehouse->getID()
        << endl;

    cout << "Name: "
        << warehouse->getName()
        << endl;


    char confirmation;

    cout << endl;

    cout << "Delete this warehouse? (y/n): ";

    cin >> confirmation;


    if (confirmation == 'y' ||
        confirmation == 'Y')
    {
        bool deleted =
            system.getWarehouseManager()
            .deleteWarehouse(id);


        if (deleted)
        {
            cout << endl;

            cout << "Warehouse deleted successfully."
                << endl;
        }
    }
    else
    {
        cout << endl;

        cout << "Delete operation cancelled."
            << endl;
    }
}

// ================================================================
// DISPLAY WAREHOUSE
// ================================================================

void ConsoleUI::displayWarehouses()
{
    cout << endl;

    cout << "========== DISPLAY WAREHOUSES =========="
        << endl;

    system.getWarehouseManager()
        .displayWarehouses();
}

// ================================================================
// *** INVENTORY MENU ***
// ================================================================

void ConsoleUI::inventoryMenu()
{
    int option;

    bool running = true;

    while (running)
    {
        cout << endl;
        cout << "========================================" << endl;
        cout << "          INVENTORY MANAGER" << endl;
        cout << "========================================" << endl;

        cout << "1. Goods Receipt" << endl;
        cout << "2. Goods Issue" << endl;
        cout << "3. Transfer Material" << endl;
        cout << "4. Inventory Check" << endl;
        cout << "5. Back" << endl;

        cout << endl;
        cout << "Select an option: ";

        cin >> option;

        switch (option)
        {
        case 1:

            goodsReceipt();

            break;

        case 2:

            goodsIssue();

            break;

        case 3:

            transferMaterial();

            break;

        case 4:

            cout << endl;
            cout << "Inventory Check is not implemented yet."
                << endl;

            break;

        case 5:

            running = false;

            break;

        default:

            cout << endl;
            cout << "Invalid option."
                << endl;

            break;
        }
    }
}

// ================================================================
// GOODS RECEIPT
// ================================================================

void ConsoleUI::goodsReceipt()
{
    int warehouseID;
    string materialID;
    int quantity;


    cout << endl;
    cout << "========== GOODS RECEIPT =========="
        << endl;


    cout << "Warehouse ID: ";
    cin >> warehouseID;


    Warehouse* warehouse =
        system.getWarehouseManager()
        .findWarehouse(warehouseID);


    if (warehouse == nullptr)
    {
        cout << endl;
        cout << "Warehouse not found."<< endl;
        return;
    }

    cout << "Material ID [###-######]: ";
    cin >> materialID;

    Material* material =
        system.getMaterialManager()
        .findMaterial(materialID);

    if (material == nullptr)
    {
        cout << endl;
        cout << "Material not found."<< endl;
        return;
    }

    cout << "Quantity received: ";
    cin >> quantity;


    if (quantity <= 0)
    {
        cout << endl;
        cout << "Quantity must be greater than zero."
            << endl;

        return;
    }


    bool success =
        system.getInventoryManager()
        .goodsReceipt(
            warehouseID,
            material,
            quantity);


    if (success)
    {
        cout << endl;
        cout << "Goods receipt completed successfully."
            << endl;
    }
    else
    {
        cout << endl;
        cout << "Goods receipt failed."
            << endl;
    }
}

// ================================================================
// GOODS ISSUE
// ================================================================

void ConsoleUI::goodsIssue()
{
    int warehouseID;
    string materialID;
    int quantity;


    cout << endl;
    cout << "========== GOODS ISSUE =========="
        << endl;


    cout << "Warehouse ID: ";
    cin >> warehouseID;


    Warehouse* warehouse =
        system.getWarehouseManager()
        .findWarehouse(warehouseID);


    if (warehouse == nullptr)
    {
        cout << endl;
        cout << "Warehouse not found."
            << endl;

        return;
    }


    cout << "Material ID [###-######]: ";
    cin >> materialID;


    WarehouseNode* node =
        warehouse->findMaterial(materialID);


    if (node == nullptr)
    {
        cout << endl;
        cout << "Material not found in warehouse."
            << endl;

        return;
    }


    cout << "Available quantity: "
        << node->quantity
        << " "
        << node->material->getUoM()
        << endl;


    cout << "Quantity to issue: ";
    cin >> quantity;


    if (quantity <= 0)
    {
        cout << endl;
        cout << "Quantity must be greater than zero."
            << endl;

        return;
    }


    bool success =
        system.getInventoryManager()
        .goodsIssue(
            warehouseID,
            materialID,
            quantity);


    if (success)
    {
        cout << endl;
        cout << "Goods issue completed successfully."
            << endl;
    }
    else
    {
        cout << endl;
        cout << "Goods issue failed."
            << endl;
        cout << "Check the available quantity."
            << endl;
    }
}

// ================================================================
// TRANSFER
// ================================================================

void ConsoleUI::transferMaterial()
{
    int sourceWarehouseID;
    int destinationWarehouseID;
    string materialID;
    int quantity;


    cout << endl;
    cout << "========== TRANSFER MATERIAL =========="
        << endl;


    cout << "Source Warehouse ID: ";
    cin >> sourceWarehouseID;


    cout << "Destination Warehouse ID: ";
    cin >> destinationWarehouseID;


    if (sourceWarehouseID == destinationWarehouseID)
    {
        cout << endl;
        cout << "Source and destination warehouses "
            << "must be different."
            << endl;

        return;
    }


    Warehouse* sourceWarehouse =
        system.getWarehouseManager()
        .findWarehouse(sourceWarehouseID);


    if (sourceWarehouse == nullptr)
    {
        cout << endl;
        cout << "Source warehouse not found."
            << endl;

        return;
    }


    Warehouse* destinationWarehouse =
        system.getWarehouseManager()
        .findWarehouse(destinationWarehouseID);


    if (destinationWarehouse == nullptr)
    {
        cout << endl;
        cout << "Destination warehouse not found."
            << endl;

        return;
    }


    cout << "Material ID [###-######]: ";
    cin >> materialID;


    WarehouseNode* node =
        sourceWarehouse->findMaterial(materialID);


    if (node == nullptr)
    {
        cout << endl;
        cout << "Material not found in source warehouse."
            << endl;

        return;
    }


    cout << "Available quantity: "
        << node->quantity
        << " "
        << node->material->getUoM()
        << endl;


    cout << "Quantity to transfer: ";
    cin >> quantity;


    if (quantity <= 0)
    {
        cout << endl;
        cout << "Quantity must be greater than zero."
            << endl;

        return;
    }


    bool success =
        system.getInventoryManager()
        .transferMaterial(
            sourceWarehouseID,
            destinationWarehouseID,
            materialID,
            quantity);


    if (success)
    {
        cout << endl;
        cout << "Material transferred successfully!"
            << endl;
    }
    else
    {
        cout << endl;
        cout << "Transfer failed."
            << endl;
    }
}

// ================================================================
// *** DATA ***
// ================================================================

void ConsoleUI::saveData()
{
    bool success =
        system.getDataManager()
        .save(
            system.getMaterialManager(),
            system.getWarehouseManager(),
            system.getProductManager());

    if (success)
    {
        cout << endl;
        cout << "Data saved successfully." << endl;
    }
    else
    {
        cout << endl;
        cout << "Error saving data." << endl;
    }
}

void ConsoleUI::loadData()
{
    bool success =
        system.getDataManager()
        .load(
            system.getMaterialManager(),
            system.getWarehouseManager(),
            system.getProductManager());

    if (success)
    {
        cout << endl;
        cout << "Data loaded successfully." << endl;
    }
    else
    {
        cout << endl;
        cout << "Error loading data." << endl;
    }
}