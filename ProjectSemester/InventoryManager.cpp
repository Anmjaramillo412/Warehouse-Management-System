#include "InventoryManager.h"


// ================================================================
// CONSTRUCTOR
// ================================================================

InventoryManager::InventoryManager(
    WarehouseManager* manager)
{
    warehouseManager = manager;
}


// ================================================================
// SET WAREHOUSE MANAGER
// ================================================================

void InventoryManager::setWarehouseManager(
    WarehouseManager* manager)
{
    warehouseManager = manager;
}


// ================================================================
// GOODS RECEIPT
// ================================================================

bool InventoryManager::goodsReceipt(
    int warehouseID,
    Material* material,
    int quantity)
{
    if (warehouseManager == nullptr)
    {
        return false;
    }

    return warehouseManager->addMaterialToWarehouse(
        warehouseID,
        material,
        quantity);
}


// ================================================================
// GOODS ISSUE
// ================================================================

bool InventoryManager::goodsIssue(
    int warehouseID,
    const string& materialID,
    int quantity)
{
    if (warehouseManager == nullptr)
    {
        return false;
    }

    return warehouseManager->removeMaterialFromWarehouse(
        warehouseID,
        materialID,
        quantity);
}


// ================================================================
// TRANSFER MATERIAL
// ================================================================

bool InventoryManager::transferMaterial(
    int sourceWarehouseID,
    int destinationWarehouseID,
    const string& materialID,
    int quantity)
{
    if (warehouseManager == nullptr)
    {
        return false;
    }


    // Source and destination must be different

    if (sourceWarehouseID == destinationWarehouseID)
    {
        return false;
    }


    // Check source warehouse

    Warehouse* sourceWarehouse =
        warehouseManager->findWarehouse(
            sourceWarehouseID);


    if (sourceWarehouse == nullptr)
    {
        return false;
    }


    // Check destination warehouse

    Warehouse* destinationWarehouse =
        warehouseManager->findWarehouse(
            destinationWarehouseID);


    if (destinationWarehouse == nullptr)
    {
        return false;
    }


    // Check material in source warehouse

    WarehouseNode* sourceNode =
        sourceWarehouse->findMaterial(
            materialID);


    if (sourceNode == nullptr)
    {
        return false;
    }


    // Check available quantity

    if (sourceNode->quantity < quantity)
    {
        return false;
    }


    // Get the Material pointer

    Material* material =
        sourceNode->material;


    // Remove from source

    bool removed =
        sourceWarehouse->removeMaterial(
            materialID,
            quantity);


    if (!removed)
    {
        return false;
    }


    // Add to destination

    bool added =
        destinationWarehouse->addMaterial(
            material,
            quantity);


    if (!added)
    {
        // IMPORTANT:
        // If destination fails, restore source.

        sourceWarehouse->addMaterial(
            material,
            quantity);

        return false;
    }


    return true;
}

// ================================================================
// SELL PRODUCT
// ================================================================

bool InventoryManager::sellProduct(
    ProductManager& productManager,
    int warehouseID,
    const string& productID,
    int quantity)
{
    if (warehouseManager == nullptr)
    {
        return false;
    }

    if (quantity <= 0)
    {
        return false;
    }


    // ============================================================
    // FIND PRODUCT
    // ============================================================

    Product* product =
        productManager.findProduct(productID);

    if (product == nullptr)
    {
        return false;
    }


    // ============================================================
    // FIND WAREHOUSE
    // ============================================================

    Warehouse* warehouse =
        warehouseManager->findWarehouse(
            warehouseID);

    if (warehouse == nullptr)
    {
        return false;
    }


    // ============================================================
    // CHECK BOM
    // ============================================================

    const vector<BOMItem>& bom =
        product->getBOM();

    if (bom.empty())
    {
        return false;
    }


    // ============================================================
    // CHECK ALL COMPONENTS BEFORE CHANGING STOCK
    // ============================================================

    for (const BOMItem& item : bom)
    {
        WarehouseNode* node =
            warehouse->findMaterial(
                item.materialID);

        if (node == nullptr)
        {
            return false;
        }


        int requiredQuantity =
            item.quantity * quantity;


        if (node->quantity < requiredQuantity)
        {
            return false;
        }
    }


    // ============================================================
    // ALL COMPONENTS AVAILABLE
    // ============================================================

    for (const BOMItem& item : bom)
    {
        int requiredQuantity =
            item.quantity * quantity;


        bool success =
            warehouse->removeMaterial(
                item.materialID,
                requiredQuantity);


        if (!success)
        {
            return false;
        }
    }


    return true;
}