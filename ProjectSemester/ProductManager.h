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

    string configFilename;

    // How many units of a Product the Warehouse stock of each BOM
    // material should be enough to build, for the Display Products
    // stock column to show that material as sufficient (green) rather
    // than short (red) at a given Warehouse. Adjustable in Product
    // Settings; defaults to 10 units.
    int safetyStockUnits;

public:

    ProductManager(
        MaterialManager* manager = nullptr,
        string configFile = "data/product_config.txt");

    void setMaterialManager(
        MaterialManager* manager);

    // Create
    bool createProduct(
        const Product& product);

    // Search
    Product* findProduct(
        const string& id);

    // Modify - name, description and the whole BOM (replaced
    // outright, the same as Create). Refuses if the Product does not
    // exist, the name is empty, or any BOM material does not exist -
    // same validation as createProduct(). The ID itself never
    // changes here.
    bool modifyProduct(
        const string& id,
        const string& name,
        const string& description,
        const vector<BOMItem>& bom);

    // Delete
    bool deleteProduct(
        const string& id);

    // Display
    void displayProducts() const;

    // Access
    const vector<unique_ptr<Product>>&
        getProducts() const;

    // ------------------------------------------------------------
    // Safety stock configuration (Product Settings)
    // ------------------------------------------------------------

    int getSafetyStockUnits() const;

    bool setSafetyStockUnits(int units);

    // ------------------------------------------------------------
    // Persistence (safety stock config only - Products themselves
    // are not persisted here)
    // ------------------------------------------------------------

    bool save();
    bool load();

    // Clear
    void clear();
};