#pragma once

#include <vector>
#include <memory>
#include <string>

#include "ProcurementOrder.h"
#include "MaterialManager.h"
#include "InventoryManager.h"
#include "MovementLogger.h"

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
    MovementLogger* movementLogger;

    // Generates the next consecutive ID, e.g. "PRC-000001"
    string generateNextID();

public:

    // Constructor
    ProcurementManager(
        MaterialManager* matManager = nullptr,
        InventoryManager* invManager = nullptr,
        MovementLogger* logger = nullptr,
        string file = "data/procurement_orders.txt");

    void setMaterialManager(
        MaterialManager* manager);

    void setInventoryManager(
        InventoryManager* manager);

    void setMovementLogger(
        MovementLogger* logger);

    // Reserves and returns a fresh "PRC-######" ID without creating
    // any order line yet. Used to register several materials under
    // one shared Procurement Order number (one action = one PRC),
    // by passing the same reserved ID as explicitID to createOrder
    // for each material/line.
    string reserveNextID();

    // Create a new Procurement Order line (status starts as
    // "Ordered"). projectionID is internal-only: which Projection
    // batch (if any) this order was registered from - see
    // ProcurementOrder. When explicitID is non-empty, it is used as
    // the order's ID instead of generating a new one - this is how
    // several materials registered in one action end up sharing the
    // same "PRC-######" number as separate lines.
    ProcurementOrder* createOrder(
        const string& productID,
        const string& materialID,
        int warehouseID,
        const string& orderDate,
        int orderedQuantity,
        const string& comment,
        const string& projectionID = "",
        const string& explicitID = "");

    // Search - a "PRC-######" ID may be shared by several lines
    // (one per material), so lookups always disambiguate by
    // materialID for a specific line. findOrder(id) alone just
    // checks whether any line with that ID exists at all.
    ProcurementOrder* findOrder(
        const string& id);

    ProcurementOrder* findOrder(
        const string& id,
        const string& materialID);

    // All lines sharing one "PRC-######" ID (one Procurement Order
    // covering several materials).
    vector<ProcurementOrder*> findOrderLines(
        const string& id);

    // Confirmation (one specific material line within an order)
    bool confirmOrder(
        const string& id,
        const string& materialID,
        const string& confirmationDate,
        int confirmedQuantity);

    // Adds a (possibly partial) receipt and performs the matching
    // Goods Receipt into the order's Warehouse (one specific
    // material line within an order).
    bool receiveOrder(
        const string& id,
        const string& materialID,
        const string& receiptDate,
        int receivedQuantity,
        const string& comment);

    // Deletes one material line outright (used when the confirmed
    // quantity is set to 0, meaning that line was never actually
    // needed). Refuses to delete a line that already has receipts
    // recorded against it - those already exist in inventory/
    // movement history and must not silently disappear from
    // Procurement.
    bool deleteOrder(
        const string& id,
        const string& materialID);

    // Closes every material line of this order that is still pending
    // (the rest was already received) - the supplier is not going to
    // deliver the remainder, so those units stop counting as pending
    // and the order moves out of Confirmed Orders into Archived
    // Orders, marked "Closed (Incomplete)" rather than "Completed".
    // Whatever was already received stays in inventory untouched.
    // Returns false if there was nothing pending left to close.
    bool closeOrder(
        const string& id);

    // Cancels every material line of this order that has not yet been
    // confirmed by the supplier (still status "Ordered"). Used from
    // Open Orders when an order will never be confirmed. Moves the
    // order to Archived Orders marked "Cancelled" instead of deleting
    // it outright, so the consecutive PRC numbering never has a gap
    // that looks like missing/lost data. Returns false if there was
    // nothing left to cancel (e.g. every line is already confirmed).
    bool cancelOrder(
        const string& id);

    // Access
    const vector<unique_ptr<ProcurementOrder>>&
        getOrders() const;

    // Persistence
    bool save();
    bool load();

    // Clear all orders (used when reloading system data)
    void clear();
};
