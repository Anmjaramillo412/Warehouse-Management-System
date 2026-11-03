#include "InventoryManager.h"


// ================================================================
// CONSTRUCTOR
// ================================================================

InventoryManager::InventoryManager(
    WarehouseManager* manager,
    MovementLogger* logger)
{
    warehouseManager = manager;
    movementLogger = logger;
    materialManager = nullptr;
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
// SET MOVEMENT LOGGER
// ================================================================

void InventoryManager::setMovementLogger(
    MovementLogger* logger)
{
    movementLogger = logger;
}

// ================================================================
// SET MATERIAL MANAGER
// ================================================================

void InventoryManager::setMaterialManager(
    MaterialManager* manager)
{
    materialManager = manager;
}

// ================================================================
// GOODS RECEIPT
// ================================================================

void InventoryManager::clearWarehouseLocks()
{
    lockedWarehouses.clear();
}

void InventoryManager::setWarehouseLocked(
    int warehouseID,
    bool locked)
{
    if (locked)
    {
        lockedWarehouses.insert(warehouseID);
    }
    else
    {
        lockedWarehouses.erase(warehouseID);
    }
}

bool InventoryManager::isWarehouseLocked(
    int warehouseID) const
{
    return lockedWarehouses.count(warehouseID) > 0;
}


bool InventoryManager::goodsReceipt(
    int warehouseID,
    Material* material,
    double quantity,
    const string& comment)
{
    if (warehouseManager == nullptr)
    {
        return false;
    }

    if (material == nullptr)
    {
        return false;
    }


    quantity = roundQty(quantity);

    if (quantity <= 0)
    {
        return false;
    }

    // No stock movement while an inventory count is open
    if (isWarehouseLocked(warehouseID))
    {
        return false;
    }

    // ------------------------------------------------------------
    // Add material to warehouse
    // ------------------------------------------------------------

    bool success =
        warehouseManager->addMaterialToWarehouse(
            warehouseID,
            material,
            quantity);


    if (!success)
    {
        return false;
    }


    // ------------------------------------------------------------
    // Log movement
    // ------------------------------------------------------------

    if (movementLogger != nullptr)
    {
        string details =
            "WH: "
            + to_string(warehouseID)
            + " | Material: "
            + material->getID()
            + " | Qty: "
            + formatQty(quantity);


        movementLogger->log(
            "GOODS RECEIPT",
            details,
            comment);
    }

    return true;
}

// ================================================================
// GOODS ISSUE
// ================================================================

bool InventoryManager::goodsIssue(
    int warehouseID,
    const string& materialID,
    double quantity,
    const string& comment)
{
    if (warehouseManager == nullptr)
    {
        return false;
    }

    quantity = roundQty(quantity);

    if (quantity <= 0)
    {
        return false;
    }

    if (isWarehouseLocked(warehouseID))
    {
        return false;
    }


    // ------------------------------------------------------------
    // Find warehouse
    // ------------------------------------------------------------

    Warehouse* warehouse =
        warehouseManager->findWarehouse(
            warehouseID);


    if (warehouse == nullptr)
    {
        return false;
    }


    // ------------------------------------------------------------
    // Check material
    // ------------------------------------------------------------

    WarehouseNode* node =
        warehouse->findMaterial(
            materialID);


    if (node == nullptr)
    {
        return false;
    }


    // ------------------------------------------------------------
    // Check available quantity
    // ------------------------------------------------------------

    if (qtyLess(node->quantity, quantity))
    {
        return false;
    }


    // ------------------------------------------------------------
    // Remove material
    // ------------------------------------------------------------

    bool success =
        warehouseManager->removeMaterialFromWarehouse(
            warehouseID,
            materialID,
            quantity);


    if (!success)
    {
        return false;
    }


    // ------------------------------------------------------------
    // Log movement
    // ------------------------------------------------------------

    if (movementLogger != nullptr)
    {
        string details =
            "WH: "
            + to_string(warehouseID)
            + " | Material: "
            + materialID
            + " | Qty: "
            + formatQty(quantity);


        movementLogger->log(
            "GOODS ISSUE",
            details,
            comment);
    }

    return true;
}


// ================================================================
// TRANSFER MATERIAL
// ================================================================

bool InventoryManager::transferMaterial(
    int sourceWarehouseID,
    int destinationWarehouseID,
    const string& materialID,
    double quantity,
    const string& comment)
{
    if (warehouseManager == nullptr)
    {
        return false;
    }


    quantity = roundQty(quantity);

    if (quantity <= 0)
    {
        return false;
    }

    if (isWarehouseLocked(sourceWarehouseID) ||
        isWarehouseLocked(destinationWarehouseID))
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

    if (qtyLess(sourceNode->quantity, quantity))
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

    // ------------------------------------------------
    // Add to destination
    // ------------------------------------------------

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

    // ------------------------------------------------
    // Log movement
    // ------------------------------------------------

    if (movementLogger != nullptr)
    {
        string details =
            "WH: "
            + to_string(sourceWarehouseID)
            + " -> WH: "
            + to_string(destinationWarehouseID)
            + " | Material: "
            + materialID
            + " | Qty: "
            + formatQty(quantity);

        movementLogger->log(
            "TRANSFER",
            details,
            comment);
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
    lastError = "";

    if (warehouseManager == nullptr)
    {
        return false;
    }

    if (quantity <= 0)
    {
        lastError = "Quantity must be greater than zero.";

        return false;
    }

    if (isWarehouseLocked(warehouseID))
    {
        lastError = "Warehouse " + to_string(warehouseID) +
            " is under an inventory count.";

        return false;
    }


    // ============================================================
    // FIND PRODUCT
    // ============================================================

    Product* product =
        productManager.findProduct(productID);

    if (product == nullptr)
    {
        lastError = "Product " + productID + " not found.";

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
        lastError = "Warehouse " + to_string(warehouseID) +
            " not found.";

        return false;
    }


    // ============================================================
    // CHECK BOM
    // ============================================================

    const vector<BOMItem>& bom =
        product->getBOM();

    if (bom.empty())
    {
        lastError = "Product " + productID +
            " has no Bill of Materials.";

        return false;
    }


    // ============================================================
    // CHECK ALL COMPONENTS BEFORE CHANGING STOCK
    // ============================================================
    // A BOM material made by an Internal Supplier (see
    // Material::hasInternalSupplier()) is self-manufactured in-house,
    // not a purchased item ever received into a Warehouse - it is
    // internal work that happens alongside the sale, not stock the
    // sale draws down. Requiring Warehouse stock of it here would
    // make it impossible to ever sell a Product built around one, so
    // it is skipped entirely (same reasoning, same pattern as
    // ProjectionManager::createProjection() skipping it from a
    // production Projection's shortfall calculation).

    for (const BOMItem& item : bom)
    {
        if (materialManager != nullptr)
        {
            Material* bomMaterial =
                materialManager->findMaterial(
                    item.materialID);

            if (bomMaterial != nullptr &&
                bomMaterial->hasInternalSupplier())
            {
                continue;
            }
        }


        WarehouseNode* node =
            warehouse->findMaterial(
                item.materialID);

        double requiredQuantity =
            roundQty(item.quantity * quantity);

        double availableQuantity =
            (node != nullptr) ? node->quantity : 0.0;


        if (qtyLess(availableQuantity, requiredQuantity))
        {
            string name = "";

            if (materialManager != nullptr)
            {
                Material* shortMaterial =
                    materialManager->findMaterial(item.materialID);

                if (shortMaterial != nullptr)
                {
                    name = " (" + shortMaterial->getName() + ")";
                }
            }

            lastError = "Not enough stock of " + item.materialID +
                name + " in Warehouse " + to_string(warehouseID) +
                ": required " + formatQty(requiredQuantity) +
                ", available " + formatQty(availableQuantity) + ".";

            return false;
        }
    }


    // ============================================================
    // ALL COMPONENTS AVAILABLE
    // ============================================================

    for (const BOMItem& item : bom)
    {
        if (materialManager != nullptr)
        {
            Material* bomMaterial =
                materialManager->findMaterial(
                    item.materialID);

            if (bomMaterial != nullptr &&
                bomMaterial->hasInternalSupplier())
            {
                continue;
            }
        }


        double requiredQuantity =
            roundQty(item.quantity * quantity);


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