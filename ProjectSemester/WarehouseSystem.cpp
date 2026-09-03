#include "WarehouseSystem.h"


// ================================================================
// CONSTRUCTOR
// ================================================================

WarehouseSystem::WarehouseSystem()
    : productManager(&materialManager), 
    inventoryManager(
        &warehouseManager,
        &movementLogger)
{
    dataManager.setMovementLogger(
        &movementLogger);
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

// ================================================================
// MOVEMENT LOGGER
// ================================================================

MovementLogger&
WarehouseSystem::getMovementLogger()
{
    return movementLogger;
}

// ================================================================
// SET LOG DATA OPERATIONS
// ================================================================

void WarehouseSystem::setLogDataOperations(
    bool enabled)
{
    movementLogger.setLogDataOperations(
        enabled);
}

// ================================================================
// GET LOG DATA OPERATIONS
// ================================================================

bool WarehouseSystem::getLogDataOperations() const
{
    return movementLogger.getLogDataOperations();
}