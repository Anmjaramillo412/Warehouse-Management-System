#include "WarehouseSystem.h"


// ================================================================
// CONSTRUCTOR
// ================================================================

WarehouseSystem::WarehouseSystem()
    : productManager(&materialManager), 
    inventoryManager(&warehouseManager)
{
}


// ================================================================
// MATERIAL MANAGER
// ================================================================

MaterialManager&
WarehouseSystem::getMaterialManager()
{
    return materialManager;
}

// ================================================================
// PRODUCT MANAGER
// ================================================================

ProductManager&
WarehouseSystem::getProductManager()
{
    return productManager;
}

// ================================================================
// WAREHOUSE MANAGER
// ================================================================

WarehouseManager&
WarehouseSystem::getWarehouseManager()
{
    return warehouseManager;
}


// ================================================================
// INVENTORY MANAGER
// ================================================================

InventoryManager&
WarehouseSystem::getInventoryManager()
{
    return inventoryManager;
}

// ================================================================
// DATA MANAGER
// ================================================================

DataManager&
WarehouseSystem::getDataManager()
{
    return dataManager;
}