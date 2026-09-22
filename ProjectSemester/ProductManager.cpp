#include "ProductManager.h"

#include <iostream>
#include <fstream>

using namespace std;


// ================================================================
// CONSTRUCTOR
// ================================================================

ProductManager::ProductManager(
    MaterialManager* manager,
    string configFile)
{
    materialManager = manager;

    configFilename = configFile;

    // 10 is a safe default (see ProductManager.h) until Product
    // Settings sets a different Safety Stock value.
    safetyStockUnits = 10;

    load();
}


// ================================================================
// SET MATERIAL MANAGER
// ================================================================

void ProductManager::setMaterialManager(
    MaterialManager* manager)
{
    materialManager = manager;
}


// ================================================================
// CREATE PRODUCT
// ================================================================

bool ProductManager::createProduct(
    const Product& product)
{
    if (product.getID().empty())
    {
        return false;
    }


    // Check if Product ID already exists

    if (findProduct(
        product.getID()) != nullptr)
    {
        return false;
    }


    // Check BOM materials

    if (materialManager != nullptr)
    {
        for (const auto& item :
            product.getBOM())
        {
            if (materialManager->findMaterial(
                item.materialID) == nullptr)
            {
                return false;
            }
        }
    }


    products.push_back(
        make_unique<Product>(product));


    return true;
}


// ================================================================
// FIND PRODUCT
// ================================================================

Product* ProductManager::findProduct(
    const string& id)
{
    for (const auto& product : products)
    {
        if (product->getID() == id)
        {
            return product.get();
        }
    }


    return nullptr;
}


// ================================================================
// DELETE PRODUCT
// ================================================================

bool ProductManager::deleteProduct(
    const string& id)
{
    for (auto it = products.begin();
        it != products.end();
        ++it)
    {
        if ((*it)->getID() == id)
        {
            products.erase(it);

            return true;
        }
    }


    return false;
}


// ================================================================
// DISPLAY PRODUCTS
// ================================================================

void ProductManager::displayProducts() const
{
    if (products.empty())
    {
        cout << endl;

        cout << "No products available."
            << endl;

        return;
    }


    cout << endl;

    cout << "========== PRODUCTS =========="
        << endl;


    for (const auto& product : products)
    {
        product->display();
    }
}


// ================================================================
// GET PRODUCTS
// ================================================================

const vector<unique_ptr<Product>>&
ProductManager::getProducts() const
{
    return products;
}


// ================================================================
// SAFETY STOCK CONFIGURATION
// ================================================================

int ProductManager::getSafetyStockUnits() const
{
    return safetyStockUnits;
}


bool ProductManager::setSafetyStockUnits(int units)
{
    if (units <= 0)
    {
        return false;
    }

    safetyStockUnits = units;

    return save();
}


// ================================================================
// SAVE
// ================================================================
// Plain text config file, a single number: the Safety Stock (in
// units of the Product) used by the Display Products stock column.

bool ProductManager::save()
{
    ofstream file(configFilename);

    if (!file.is_open())
    {
        return false;
    }

    file << safetyStockUnits << endl;

    file.close();

    return true;
}


// ================================================================
// LOAD
// ================================================================

bool ProductManager::load()
{
    ifstream file(configFilename);

    if (!file.is_open())
    {
        // No file yet - not an error, just keep the default.

        return true;
    }

    int units = 0;

    file >> units;

    if (units > 0)
    {
        safetyStockUnits = units;
    }

    file.close();

    return true;
}


// ================================================================
// CLEAR
// ================================================================

void ProductManager::clear()
{
    products.clear();
}