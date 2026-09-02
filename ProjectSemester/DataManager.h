#pragma once

#include "MaterialManager.h"
#include "WarehouseManager.h"
#include "ProductManager.h"
#include "MovementLogger.h"

#include <string>

using namespace std;

class DataManager
{
private:

    string filename;
    MovementLogger* movementLogger;

public:

    // Constructor
    DataManager(
        string file = "warehouse_data.xlsx");

    void setMovementLogger(
        MovementLogger* logger);

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
