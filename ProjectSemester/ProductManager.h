#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Product.h"
#include "MaterialManager.h"

using namespace std;

class ProductManager
{
private:

    vector<unique_ptr<Product>> products;

    MaterialManager* materialManager;

public:

    ProductManager(
        MaterialManager* manager = nullptr);

    void setMaterialManager(
        MaterialManager* manager);

    // Create
    bool createProduct(
        const Product& product);

    // Search
    Product* findProduct(
        const string& id);

    // Delete
    bool deleteProduct(
        const string& id);

    // Display
    void displayProducts() const;

    // Access
    const vector<unique_ptr<Product>>&
        getProducts() const;

    // Clear
    void clear();
};