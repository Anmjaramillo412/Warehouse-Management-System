#include "ConsoleUI.h"

#include <iostream>
#include <string>
#include <limits>
#include <vector>

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
        cout << "4. Supplier Manager" << endl;
        cout << "5. Save Test" << endl;
        cout << "6. Exit" << endl;

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

            supplierMenu();

            break;

        case 5:

            saveData();

            break;

        case 6:

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
// *** SUPPLIER MENU ***
// ================================================================

void ConsoleUI::supplierMenu()
{
    int option;

    bool running = true;

    while (running)
    {
        cout << endl;
        cout << "========================================" << endl;
        cout << "          SUPPLIER MANAGER" << endl;
        cout << "========================================" << endl;

        cout << "1. Create Supplier" << endl;
        cout << "2. Modify Supplier" << endl;
        cout << "3. Display Suppliers" << endl;
        cout << "4. Back" << endl;

        cout << endl;
        cout << "Select an option: ";

        cin >> option;

        switch (option)
        {
        case 1:

            createSupplier();

            break;

        case 2:

            modifySupplier();

            break;

        case 3:

            displaySuppliers();

            break;

        case 4:

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
// CREATE SUPPLIER
// ================================================================

void ConsoleUI::createSupplier()
{
    string name;
    string address;
    string country;
    string contactName;
    string contactEmail;
    string website;

    vector<string> orderingMethods;

    string paymentMethod;
    int leadTimeWeeks;


    cout << endl;
    cout << "========== CREATE SUPPLIER =========="
        << endl;

    cin.ignore(
        numeric_limits<streamsize>::max(),
        '\n'
    );

    cout << "Name: ";
    getline(cin, name);

    cout << "Address: ";
    getline(cin, address);

    cout << "Country: ";
    getline(cin, country);

    cout << "Contact Name: ";
    getline(cin, contactName);

    cout << "Contact Email: ";
    getline(cin, contactEmail);

    cout << "Website: ";
    getline(cin, website);


    // ------------------------------------------------------------
    // Ordering methods (multiple selection)
    // ------------------------------------------------------------

    char answer;

    cout << "Ordering method - Online? (y/n): ";
    cin >> answer;
    if (answer == 'y' || answer == 'Y')
    {
        orderingMethods.push_back("Online");
    }

    cout << "Ordering method - Email? (y/n): ";
    cin >> answer;
    if (answer == 'y' || answer == 'Y')
    {
        orderingMethods.push_back("Email");
    }

    cout << "Ordering method - PO? (y/n): ";
    cin >> answer;
    if (answer == 'y' || answer == 'Y')
    {
        orderingMethods.push_back("PO");
    }


    // ------------------------------------------------------------
    // Payment method (single selection)
    // ------------------------------------------------------------

    int paymentOption = 0;

    cout << endl;
    cout << "Payment Method:" << endl;
    cout << "1. Invoice" << endl;
    cout << "2. Pre-payment" << endl;
    cout << "3. Online" << endl;
    cout << "Select an option: ";
    cin >> paymentOption;

    switch (paymentOption)
    {
    case 2:
        paymentMethod = "Pre-payment";
        break;

    case 3:
        paymentMethod = "Online";
        break;

    case 1:
    default:
        paymentMethod = "Invoice";
        break;
    }


    cout << "Lead Time (weeks): ";
    cin >> leadTimeWeeks;


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


    bool created =
        system.getSupplierManager()
        .createSupplier(supplier);


    if (created)
    {
        cout << endl;
        cout << "Supplier created successfully!"
            << endl;
    }
    else
    {
        cout << endl;
        cout << "ERROR: Supplier name is empty "
            << "or already exists!"
            << endl;
    }
}

// ================================================================
// MODIFY SUPPLIER
// ================================================================

void ConsoleUI::modifySupplier()
{
    string name;

    cout << endl;
    cout << "========== MODIFY SUPPLIER =========="
        << endl;

    cin.ignore(
        numeric_limits<streamsize>::max(),
        '\n'
    );

    cout << "Enter Supplier Name: ";

    getline(cin, name);


    Supplier* supplier =
        system.getSupplierManager()
        .findSupplier(name);


    if (supplier == nullptr)
    {
        cout << endl;
        cout << "Supplier not found."
            << endl;

        return;
    }


    string address;
    string country;
    string contactName;
    string contactEmail;
    string website;

    vector<string> orderingMethods;

    string paymentMethod;
    int leadTimeWeeks;


    // ============================================================
    // ADDRESS
    // ============================================================

    cout << endl;

    cout << "Current Address: "
        << supplier->getAddress()
        << endl;

    cout << "New Address: ";

    getline(cin, address);

    if (address.empty())
    {
        address = supplier->getAddress();
    }


    // ============================================================
    // COUNTRY
    // ============================================================

    cout << endl;

    cout << "Current Country: "
        << supplier->getCountry()
        << endl;

    cout << "New Country: ";

    getline(cin, country);

    if (country.empty())
    {
        country = supplier->getCountry();
    }


    // ============================================================
    // CONTACT NAME
    // ============================================================

    cout << endl;

    cout << "Current Contact Name: "
        << supplier->getContactName()
        << endl;

    cout << "New Contact Name: ";

    getline(cin, contactName);

    if (contactName.empty())
    {
        contactName = supplier->getContactName();
    }


    // ============================================================
    // CONTACT EMAIL
    // ============================================================

    cout << endl;

    cout << "Current Contact Email: "
        << supplier->getContactEmail()
        << endl;

    cout << "New Contact Email: ";

    getline(cin, contactEmail);

    if (contactEmail.empty())
    {
        contactEmail = supplier->getContactEmail();
    }


    // ============================================================
    // WEBSITE
    // ============================================================

    cout << endl;

    cout << "Current Website: "
        << supplier->getWebsite()
        << endl;

    cout << "New Website: ";

    getline(cin, website);

    if (website.empty())
    {
        website = supplier->getWebsite();
    }


    // ============================================================
    // ORDERING METHODS
    // ============================================================

    char changeOrdering;

    cout << endl;

    cout << "Change Ordering Methods? (y/n): ";

    cin >> changeOrdering;

    if (changeOrdering == 'y' || changeOrdering == 'Y')
    {
        char answer;

        cout << "Ordering method - Online? (y/n): ";
        cin >> answer;
        if (answer == 'y' || answer == 'Y')
        {
            orderingMethods.push_back("Online");
        }

        cout << "Ordering method - Email? (y/n): ";
        cin >> answer;
        if (answer == 'y' || answer == 'Y')
        {
            orderingMethods.push_back("Email");
        }

        cout << "Ordering method - PO? (y/n): ";
        cin >> answer;
        if (answer == 'y' || answer == 'Y')
        {
            orderingMethods.push_back("PO");
        }
    }
    else
    {
        orderingMethods = supplier->getOrderingMethods();
    }


    // ============================================================
    // PAYMENT METHOD
    // ============================================================

    char changePayment;

    cout << endl;

    cout << "Current Payment Method: "
        << supplier->getPaymentMethod()
        << endl;

    cout << "Change Payment Method? (y/n): ";

    cin >> changePayment;

    if (changePayment == 'y' || changePayment == 'Y')
    {
        int paymentOption = 0;

        cout << "Payment Method:" << endl;
        cout << "1. Invoice" << endl;
        cout << "2. Pre-payment" << endl;
        cout << "3. Online" << endl;
        cout << "Select an option: ";
        cin >> paymentOption;

        switch (paymentOption)
        {
        case 2:
            paymentMethod = "Pre-payment";
            break;

        case 3:
            paymentMethod = "Online";
            break;

        case 1:
        default:
            paymentMethod = "Invoice";
            break;
        }
    }
    else
    {
        paymentMethod = supplier->getPaymentMethod();
    }


    // ============================================================
    // LEAD TIME
    // ============================================================

    cout << endl;

    cout << "Current Lead Time: "
        << supplier->getLeadTimeWeeks()
        << " week(s)"
        << endl;

    cout << "New Lead Time in weeks "
        << "(enter -1 to keep current): ";

    cin >> leadTimeWeeks;

    if (leadTimeWeeks < 0)
    {
        leadTimeWeeks = supplier->getLeadTimeWeeks();
    }


    // ============================================================
    // MODIFY
    // ============================================================

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


    bool modified =
        system.getSupplierManager()
        .modifySupplier(
            name,
            updatedSupplier);


    if (modified)
    {
        cout << endl;
        cout << "Supplier successfully modified."
            << endl;
    }
    else
    {
        cout << endl;
        cout << "Supplier could not be modified."
            << endl;
    }
}

// ================================================================
// DISPLAY SUPPLIERS
// ================================================================

void ConsoleUI::displaySuppliers()
{
    cout << endl;
    cout << "========== DISPLAY SUPPLIERS =========="
        << endl;

    system.getSupplierManager()
        .displaySuppliers();
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

MaterialType ConsoleUI::readMaterialType()
{
    int typeOption = 0;

    cout << endl;
    cout << "Material Type:" << endl;
    cout << "1. Standard Part" << endl;
    cout << "2. Design Part" << endl;
    cout << "3. PCB" << endl;
    cout << "Select an option: ";
    cin >> typeOption;

    switch (typeOption)
    {
    case 2:
        return MaterialType::DesignPart;

    case 3:
        return MaterialType::PCB;

    case 1:
    default:
        return MaterialType::StandardPart;
    }
}

Material ConsoleUI::readMaterial()
{
    string id;
    string name;
    string description;
    string uom;
    string category;
    string drawingNumber;
    string manufacturer;
    string manufacturerPartNumber;
    string supplierName;
    string supplierPartNumber;
    string photoPath;
    bool active;


    // ID

    cout << endl;
    cout << "Material ID [###-###### or ######-00]: ";

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


    // Material Type

    MaterialType type =
        readMaterialType();


    // Drawing Number
    // (only for Design Part / PCB)

    if (Material::requiresDrawingNumber(type))
    {
        do
        {
            cout << "Drawing Number "
                << "[###-ASM-####, ###-PAR-####, "
                << "I-BU#-##, OWI-BU#-##, ###-PCB-####]: ";

            cin >> drawingNumber;

        } while (!Material::isValidDrawingNumber(drawingNumber));

        cin.ignore(
            numeric_limits<streamsize>::max(),
            '\n'
        );
    }


    // Manufacturer

    cout << "Manufacturer: ";

    getline(cin, manufacturer);


    // Manufacturer Part Number

    cout << "Manufacturer Part Number: ";

    getline(cin, manufacturerPartNumber);


    // Supplier
    // (must already exist - see Supplier Manager)

    Supplier* supplier = nullptr;

    do
    {
        cout << "Supplier name "
            << "(must already exist): ";

        getline(cin, supplierName);

        supplier =
            system.getSupplierManager()
            .findSupplier(supplierName);

        if (supplier == nullptr)
        {
            cout << endl;
            cout << "Supplier not found. "
                << "Create it first from the Supplier Manager."
                << endl;
        }

    } while (supplier == nullptr);


    // Supplier Part Number

    cout << "Supplier Part Number: ";

    getline(cin, supplierPartNumber);


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
        type,
        drawingNumber,
        manufacturer,
        manufacturerPartNumber,
        supplier,
        supplierPartNumber,
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
    string drawingNumber;
    string manufacturer;
    string manufacturerPartNumber;
    string supplierName;
    string supplierPartNumber;
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
    // TYPE / DRAWING NUMBER
    // ============================================================

    cout << endl;

    cout << "Current Type: "
        << Material::materialTypeToString(material->getType())
        << endl;

    cout << "Current Drawing Number: "
        << material->getDrawingNumber()
        << endl;

    cout << "Change Material Type? (y/n): ";

    char changeType;
    cin >> changeType;

    MaterialType type =
        material->getType();

    if (changeType == 'y' || changeType == 'Y')
    {
        type = readMaterialType();
    }

    if (Material::requiresDrawingNumber(type))
    {
        do
        {
            cout << "Drawing Number "
                << "[###-ASM-####, ###-PAR-####, "
                << "I-BU#-##, OWI-BU#-##, ###-PCB-####]: ";

            cin >> drawingNumber;

        } while (!Material::isValidDrawingNumber(drawingNumber));
    }
    else
    {
        drawingNumber = "";
    }

    cin.ignore(
        numeric_limits<streamsize>::max(),
        '\n'
    );


    // ============================================================
    // MANUFACTURER
    // ============================================================

    cout << endl;

    cout << "Current Manufacturer: "
        << material->getManufacturer()
        << endl;

    cout << "New Manufacturer: ";

    getline(cin, manufacturer);

    if (manufacturer.empty())
    {
        manufacturer = material->getManufacturer();
    }


    // ============================================================
    // MANUFACTURER PART NUMBER
    // ============================================================

    cout << endl;

    cout << "Current Manufacturer Part Number: "
        << material->getManufacturerPartNumber()
        << endl;

    cout << "New Manufacturer Part Number: ";

    getline(cin, manufacturerPartNumber);

    if (manufacturerPartNumber.empty())
    {
        manufacturerPartNumber =
            material->getManufacturerPartNumber();
    }


    // ============================================================
    // SUPPLIER
    // ============================================================

    Supplier* currentSupplier =
        material->getSupplier();

    cout << endl;

    cout << "Current Supplier: "
        << (currentSupplier != nullptr
            ? currentSupplier->getName()
            : "N/A")
        << endl;

    cout << "New Supplier name "
        << "(leave blank to keep current, must exist): ";

    getline(cin, supplierName);

    Supplier* supplier = currentSupplier;

    if (!supplierName.empty())
    {
        supplier =
            system.getSupplierManager()
            .findSupplier(supplierName);

        if (supplier == nullptr)
        {
            cout << endl;
            cout << "Supplier not found. "
                << "Keeping the current supplier."
                << endl;

            supplier = currentSupplier;
        }
    }


    // ============================================================
    // SUPPLIER PART NUMBER
    // ============================================================

    cout << endl;

    cout << "Current Supplier Part Number: "
        << material->getSupplierPartNumber()
        << endl;

    cout << "New Supplier Part Number: ";

    getline(cin, supplierPartNumber);

    if (supplierPartNumber.empty())
    {
        supplierPartNumber =
            material->getSupplierPartNumber();
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
        type,
        drawingNumber,
        manufacturer,
        manufacturerPartNumber,
        supplier,
        supplierPartNumber,
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
            system.getSupplierManager(),
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
            system.getSupplierManager(),
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