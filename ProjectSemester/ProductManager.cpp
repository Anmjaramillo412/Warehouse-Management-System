#include "ProductManager.h"

#include <iostream>

using namespace std;


// ================================================================
// CONSTRUCTOR
// ================================================================

ProductManager::ProductManager(
    MaterialManager* manager)
{
    materialManager = manager;
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
// CLEAR
// ================================================================

void ProductManager::clear()
{
    products.clear();
}