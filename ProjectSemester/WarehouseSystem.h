#pragma once

#include "MaterialManager.h"
#include "SupplierManager.h"
#include "ProductManager.h"
#include "WarehouseManager.h"
#include "MovementLogger.h"
#include "InventoryManager.h"
#include "DataManager.h"
#include "ProcurementManager.h"
#include "ProjectionManager.h"
#include "PurchaseManager.h"
#include "InventoryCountManager.h"

class WarehouseSystem
{
private:

    MaterialManager materialManager;

    SupplierManager supplierManager;

    ProductManager productManager;

    WarehouseManager warehouseManager;

    MovementLogger movementLogger;

    InventoryManager inventoryManager;

    DataManager dataManager;

    // Declared last: its constructor takes pointers to
    // materialManager and inventoryManager above, which must
    // already be constructed first.
    ProcurementManager procurementManager;

    // Also declared last, for the same reason: it takes pointers to
    // productManager, warehouseManager and movementLogger above.
    ProjectionManager projectionManager;

    // Declared last too: it takes pointers to procurementManager and
    // movementLogger above, which must already be constructed first.
    PurchaseManager purchaseManager;

    // Declared last as well: it takes pointers to warehouseManager,
    // materialManager, inventoryManager and movementLogger above.
    InventoryCountManager inventoryCountManager;

public:

    WarehouseSystem();

    MaterialManager& getMaterialManager();

    SupplierManager& getSupplierManager();

    ProductManager& getProductManager();

    WarehouseManager& getWarehouseManager();

    MovementLogger& getMovementLogger();

    InventoryManager& getInventoryManager();

    DataManager& getDataManager();

    ProcurementManager& getProcurementManager();

    ProjectionManager& getProjectionManager();

    PurchaseManager& getPurchaseManager();

    InventoryCountManager& getInventoryCountManager();

    void setLogDataOperations(
        bool enabled);

    bool getLogDataOperations() const;
};