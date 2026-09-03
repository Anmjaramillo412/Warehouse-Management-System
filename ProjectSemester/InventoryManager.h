#pragma once

#include "WarehouseManager.h"
#include "ProductManager.h"
#include "MovementLogger.h"

using namespace std;

class InventoryManager
{
private:

    WarehouseManager* warehouseManager;
    MovementLogger* movementLogger;

public:

    InventoryManager(
        WarehouseManager* manager = nullptr,
        MovementLogger* logger = nullptr);

    void setWarehouseManager(
        WarehouseManager* manager);

    void setMovementLogger(
        MovementLogger* logger);

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

