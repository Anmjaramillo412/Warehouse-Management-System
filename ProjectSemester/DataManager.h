#pragma once

#include "MaterialManager.h"
#include "WarehouseManager.h"
#include "ProductManager.h"

#include <string>

using namespace std;

class DataManager
{
private:

    string filename;

public:

    // Constructor
    DataManager(
        string file = "warehouse_data.xlsx");

    // Save
    bool save(
        MaterialManager& materialManager,
        WarehouseManager& warehouseManager,
        ProductManager& productManager);

    // Load
    bool load(
        MaterialManager& materialManager,
        WarehouseManager& warehouseManager,
        ProductManager& productManager);
};
