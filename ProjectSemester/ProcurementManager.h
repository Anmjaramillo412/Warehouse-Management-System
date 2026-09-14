#pragma once

#include <vector>
#include <memory>
#include <string>

#include "ProcurementOrder.h"
#include "MaterialManager.h"
#include "InventoryManager.h"

using namespace std;


// ================================================================
// PROCUREMENT MANAGER
// ================================================================
// Owns every Procurement Order and its own persistence file
// (kept separate from warehouse_data.xlsx - see save()/load()).
// Every mutating call (create/confirm/receive) saves immediately,
// the same way MovementLogger writes on every movement, so no
// Procurement data depends on the manual Save Data button.

class ProcurementManager
{
private:

    vector<unique_ptr<ProcurementOrder>> orders;

    string filename;

    int nextNumber;

    MaterialManager* materialManager;
    InventoryManager* inventoryManager;

    // Generates the next consecutive ID, e.g. "PRC-000001"
    string generateNextID();

public:

    // Constructor
    ProcurementManager(
        MaterialManager* matManager = nullptr,
        InventoryManager* invManager = nullptr,
        string file = "data/procurement_orders.txt");

    void setMaterialManager(
        MaterialManager* manager);

    void setInventoryManager(
        InventoryManager* manager);

    // Create a new Procurement Order (status starts as "Ordered")
    ProcurementOrder* createOrder(
        const string& productID,
        const string& materialID,
        int warehouseID,
        const string& orderDate,
        int orderedQuantity,
        const string& comment);

    // Search
    ProcurementOrder* findOrder(
        const string& id);

    // Confirmation
    bool confirmOrder(
        const string& id,
        const string& confirmationDate,
        int confirmedQuantity);

    // Adds a (possibly partial) receipt and performs the matching
    // Goods Receipt into the order's Warehouse.
    bool receiveOrder(
        const string& id,
        const string& receiptDate,
        int receivedQuantity,
        const string& comment);

    // Access
    const vector<unique_ptr<ProcurementOrder>>&
        getOrders() const;

    // Persistence
    bool save();
    bool load();

    // Clear all orders (used when reloading system data)
    void clear();
};
