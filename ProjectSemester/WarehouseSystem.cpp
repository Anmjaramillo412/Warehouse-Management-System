#include "WarehouseSystem.h"


// ================================================================
// CONSTRUCTOR
// ================================================================

WarehouseSystem::WarehouseSystem()
    : productManager(&materialManager),
    inventoryManager(
        &warehouseManager,
        &movementLogger),
    procurementManager(
        &materialManager,
        &inventoryManager,
        &movementLogger),
    projectionManager(
        &productManager,
        &warehouseManager,
        &inventoryManager,
        &movementLogger,
        "data/projections.txt",
        &materialManager),
    purchaseManager(
        &procurementManager,
        &movementLogger)
{
    dataManager.setMovementLogger(
        &movementLogger);

    // So sellProduct() can skip BOM materials made by an Internal
    // Supplier (see InventoryManager.h) - not passed as a constructor
    // argument above because inventoryManager's constructor predates
    // this need and other code already calls it with just
    // (warehouseManager, movementLogger) elsewhere.
    inventoryManager.setMaterialManager(
        &materialManager);

    // So "Other Costs" amortization can resolve each Purchase Invoice
    // line's material to its Supplier (see PurchaseManager.h) - same
    // reason/pattern as inventoryManager above.
    purchaseManager.setMaterialManager(
        &materialManager);
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
// SUPPLIER MANAGER
// ================================================================

SupplierManager&
WarehouseSystem::getSupplierManager()
{
    return supplierManager;
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
// PROCUREMENT MANAGER
// ================================================================

ProcurementManager&
WarehouseSystem::getProcurementManager()
{
    return procurementManager;
}

// ================================================================
// PROJECTION MANAGER
// ================================================================

ProjectionManager&
WarehouseSystem::getProjectionManager()
{
    return projectionManager;
}

// ================================================================
// PURCHASE MANAGER
// ================================================================

PurchaseManager&
WarehouseSystem::getPurchaseManager()
{
    return purchaseManager;
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