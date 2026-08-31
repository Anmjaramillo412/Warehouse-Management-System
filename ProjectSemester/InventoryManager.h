#pragma once

#include "WarehouseManager.h"
#include "ProductManager.h"

using namespace std;

class InventoryManager
{
private:

    WarehouseManager* warehouseManager;

public:

    InventoryManager(
        WarehouseManager* manager = nullptr);

    void setWarehouseManager(
        WarehouseManager* manager);

    // Goods Receipt
    bool goodsReceipt(
        int warehouseID,
        Material* material,
        int quantity);

    // Goods Issue
    bool goodsIssue(
        int warehouseID,
        const string& materialID,
        int quantity);

    // Transfer
    bool transferMaterial(
        int sourceWarehouseID,
        int destinationWarehouseID,
        const string& materialID,
        int quantity);

    // Sell Product
    bool sellProduct(
        ProductManager& productManager,
        int warehouseID,
        const string& productID,
        int quantity);
};