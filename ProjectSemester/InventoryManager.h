#pragma once

#include "WarehouseManager.h"
#include "ProductManager.h"
#include "MovementLogger.h"
#include "MaterialManager.h"

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

public:

    InventoryManager(
        WarehouseManager* manager = nullptr,
        MovementLogger* logger = nullptr);

    void setWarehouseManager(
        WarehouseManager* manager);

    void setMovementLogger(
        MovementLogger* logger);

    void setMaterialManager(
        MaterialManager* manager);

    // Goods Receipt
    bool goodsReceipt(
        int warehouseID,
        Material* material,
        int quantity,
        const string& comment = "");

    // Goods Issue
    bool goodsIssue(
        int warehouseID,
        const string& materialID,
        int quantity,
        const string& comment = "");

    // Transfer
    bool transferMaterial(
        int sourceWarehouseID,
        int destinationWarehouseID,
        const string& materialID,
        int quantity,
        const string& comment = "");

    // Sell Product
    bool sellProduct(
        ProductManager& productManager,
        int warehouseID,
        const string& productID,
        int quantity);
};

