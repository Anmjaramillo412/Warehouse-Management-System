#pragma once

#include "MaterialManager.h"
#include "ProductManager.h"
#include "WarehouseManager.h"
#include "MovementLogger.h"
#include "InventoryManager.h"
#include "DataManager.h"

class WarehouseSystem
{
private:

    MaterialManager materialManager;

    ProductManager productManager;

    WarehouseManager warehouseManager;

    MovementLogger movementLogger;

    InventoryManager inventoryManager;

    DataManager dataManager;

public:

    WarehouseSystem();

    MaterialManager& getMaterialManager();

    ProductManager& getProductManager();

    WarehouseManager& getWarehouseManager();

    MovementLogger& getMovementLogger();

    InventoryManager& getInventoryManager();

    DataManager& getDataManager();

    void setLogDataOperations(
        bool enabled);

    bool getLogDataOperations() const;
};