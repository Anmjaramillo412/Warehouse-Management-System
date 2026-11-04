#pragma once

#include "WarehouseManager.h"
#include "ProductManager.h"
#include "MovementLogger.h"
#include "MaterialManager.h"

#include <set>

using namespace std;

class InventoryManager
{
private:

    WarehouseManager* warehouseManager;
    MovementLogger* movementLogger;

    // Only needed by sellProduct(), to skip BOM materials made by an
    // Internal Supplier (see Material::hasInternalSupplier()) - those
    // are self-manufactured in-house, never received into a Warehouse
    // as purchased stock, so requiring warehouse stock of them would
    // make it impossible to ever sell a Product that uses one. Same
    // reasoning, same pattern as ProjectionManager::materialManager.
    MaterialManager* materialManager;

    // Warehouses with an Open inventory count (see
    // InventoryCountManager.h): while a count is open, no stock may
    // move in or out of that Warehouse - the counted quantities have
    // to stay comparable to the system quantities until the count is
    // applied or cancelled.
    set<int> lockedWarehouses;

    // Why the last sellProduct() failed (empty after a success)
    string lastError;

    // Writes one stock movement of a material to the movement ledger
    // (balance = stock left in that Warehouse right after it).
    void ledgerStock(
        const string& type,
        int warehouseID,
        const string& materialID,
        double qtyIn,
        double qtyOut,
        const string& reference,
        const string& comment);

public:

    const string& getLastError() const
    {
        return lastError;
    }

    InventoryManager(
        WarehouseManager* manager = nullptr,
        MovementLogger* logger = nullptr);

    void setWarehouseManager(
        WarehouseManager* manager);

    void setMovementLogger(
        MovementLogger* logger);

    void setMaterialManager(
        MaterialManager* manager);

    // Inventory count lock
    void clearWarehouseLocks();

    void setWarehouseLocked(
        int warehouseID,
        bool locked);

    bool isWarehouseLocked(
        int warehouseID) const;

    // Goods Receipt
    // reference: e.g. the Procurement Order ID, for the movement ledger
    bool goodsReceipt(
        int warehouseID,
        Material* material,
        double quantity,
        const string& comment = "",
        const string& reference = "");

    // Goods Issue
    // type / reference: how the movement shows in the movement ledger
    // (e.g. "PRODUCTION CONSUMPTION" with the Projection ID)
    bool goodsIssue(
        int warehouseID,
        const string& materialID,
        double quantity,
        const string& comment = "",
        const string& reference = "",
        const string& type = "GOODS ISSUE");

    // Transfer
    bool transferMaterial(
        int sourceWarehouseID,
        int destinationWarehouseID,
        const string& materialID,
        double quantity,
        const string& comment = "");

    // Sell Product
    bool sellProduct(
        ProductManager& productManager,
        int warehouseID,
        const string& productID,
        int quantity,
        const string& comment = "");
};

