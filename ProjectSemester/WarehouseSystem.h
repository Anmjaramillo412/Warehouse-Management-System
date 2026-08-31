#pragma once

#include "MaterialManager.h"
#include "ProductManager.h"
#include "WarehouseManager.h"
#include "InventoryManager.h"
#include "DataManager.h"

class WarehouseSystem
{
private:

    MaterialManager materialManager;

    ProductManager productManager;

    WarehouseManager warehouseManager;

    InventoryManager inventoryManager;

    DataManager dataManager;

public:

    WarehouseSystem();

    MaterialManager& getMaterialManager();

    ProductManager& getProductManager();

    WarehouseManager& getWarehouseManager();

    InventoryManager& getInventoryManager();

    DataManager& getDataManager();
};