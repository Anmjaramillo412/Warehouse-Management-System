#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Projection.h"
#include "ProductManager.h"
#include "WarehouseManager.h"
#include "InventoryManager.h"
#include "MovementLogger.h"
#include "MaterialManager.h"

using namespace std;


// ================================================================
// PROJECTION MANAGER
// ================================================================
// Owns every Production Projection batch and its own persistence
// file (kept separate from warehouse_data.xlsx - see save()/load(),
// same approach as ProcurementManager). A projection freezes the
// required-vs-in-stock numbers at creation time; it does not track
// ordered/pending quantities itself - that is computed by joining
// with ProcurementManager's orders (see WebServer.cpp), since an
// order only needs to reference this batch, not the other way
// around.

class ProjectionManager
{
private:

    vector<unique_ptr<Projection>> projections;

    string filename;

    int nextNumber;

    ProductManager* productManager;
    WarehouseManager* warehouseManager;
    InventoryManager* inventoryManager;
    MovementLogger* movementLogger;
    MaterialManager* materialManager;

    // Generates the next consecutive ID, e.g. "PRJ-000001"
    string generateNextID();

    // Sum of this material's TRUE requirement, computed live from each
    // Product's BOM, across every currently open Projection (not
    // completed, deadline today or later, or no deadline at all) tied
    // to this ONE Warehouse - but ONLY those whose OWN deadline is AT
    // OR BEFORE asOfDeadline (a Projection accounts for every open
    // Projection that expires no later than it does, but a Projection
    // with a LATER delivery date must never affect one that expires
    // sooner). No deadline at all is treated as the furthest-out,
    // least-urgent date possible for this comparison only - see
    // effectiveDeadlineForOrdering() in the .cpp.
    //
    // Deliberately NOT summed from each Projection's own recorded
    // items list: an item is only recorded once a Projection's OWN
    // share already crosses the shortfall line on its own, so two or
    // more Projections that are each individually fine but combined
    // exceed stock would otherwise never surface at all. Reading the
    // live BOM instead means the true combined demand (from every
    // Projection due at or before asOfDeadline) always counts, whether
    // or not any single Projection has "caught up" to it yet.
    int getOpenRequiredQuantity(
        const string& materialID,
        int warehouseID,
        const string& asOfDeadline) const;

public:

    // Actual current stock of one material inside one Warehouse right
    // now - no Projection reservations subtracted (see
    // getQuantityToOrder() for that).
    int getWarehouseStock(
        const string& materialID,
        int warehouseID) const;

    // "Qty to Order" for one material inside one Warehouse, AS SEEN BY
    // A PROJECTION DUE ON asOfDeadline: the combined requirement of
    // every currently open Projection tied to that Warehouse whose own
    // deadline is AT OR BEFORE asOfDeadline (this necessarily includes
    // the Projection asking, since its own deadline is asOfDeadline by
    // definition - never a Projection due LATER, see
    // getOpenRequiredQuantity() above) minus that Warehouse's actual
    // current stock, floored at 0.
    //
    // This replaces the old per-Projection "Virtual Stock" model
    // (total stock minus every OTHER open Projection's reservation,
    // computed independently for each Projection), which double-
    // counted a scarce material whenever two or more Projections
    // competed for it: each one excluded only itself and so each
    // concluded, on its own, that it needed to cover the full
    // combined shortfall a second time. It is intentionally NOT one
    // single number shared identically by every open Projection any
    // more: a Projection due sooner must stay unaffected by one due
    // later - only a Projection due later is expected to account for
    // everything already due ahead of it.
    int getQuantityToOrder(
        const string& materialID,
        int warehouseID,
        const string& asOfDeadline) const;

    // Constructor
    ProjectionManager(
        ProductManager* prodManager = nullptr,
        WarehouseManager* whManager = nullptr,
        InventoryManager* invManager = nullptr,
        MovementLogger* logger = nullptr,
        string file = "data/projections.txt",
        MaterialManager* matManager = nullptr);

    void setProductManager(
        ProductManager* manager);

    void setWarehouseManager(
        WarehouseManager* manager);

    void setInventoryManager(
        InventoryManager* manager);

    void setMovementLogger(
        MovementLogger* logger);

    void setMaterialManager(
        MaterialManager* manager);

    // Creates a new Projection from the Product's BOM, the quantity
    // to manufacture, and current stock (only materials with a
    // shortfall are included). The Warehouse is no longer chosen by
    // the caller - it is always the Product's own Main Warehouse, so
    // stock is checked against that one Warehouse alone rather than
    // summed across every Warehouse. Returns nullptr - with
    // errorMessage explaining why - if the Product does not exist,
    // has no BOM, has no Main Warehouse assigned yet, or nothing is
    // actually short.
    Projection* createProjection(
        const string& productID,
        const string& deadline,
        int manufactureQuantity,
        const string& creationDate,
        string& errorMessage);

    // Records one order-placement attempt against a Projection item
    // (see Projection::registerOrderAttempt()) and saves immediately.
    // fullyCovered should be true when this attempt's registered
    // quantity, combined with whatever was already on order, meets or
    // exceeds the item's requiredQuantity - the item freezes
    // immediately in that case; otherwise it freezes only once this
    // was its second attempt. Returns false if the Projection or the
    // item is not found.
    bool registerItemOrderAttempt(
        const string& projectionID,
        const string& materialID,
        bool fullyCovered);

    // Re-scans this Projection's Product BOM for any material that
    // has newly become short (live) since this Projection was created
    // or last refreshed, and adds it as a proper item if so - using
    // the exact same shortfall math as createProjection() (this
    // Projection's own requirement plus every other open Projection
    // whose deadline is AT OR BEFORE its own - never one due later,
    // see getOpenRequiredQuantity() above - for that material/
    // Warehouse, minus actual stock).
    //
    // Without this, a material whose stock was comfortable when the
    // Projection was created but has since been consumed (by another
    // Projection, a Sale, a Goods Issue - anything) would never
    // surface as needing an order from ANY open Projection, because
    // membership was otherwise decided once, at creation time, and
    // frozen from then on. WebServer.cpp calls this every time an
    // open Projection is read (list or detail), so the item list
    // stays current without ever removing a line once added (removal
    // would lose its orderRegistered/orderRegistrationCount history).
    //
    // No-op on a completed Projection or one that cannot be found.
    // Returns true if anything was added.
    bool refreshShortfallItems(
        const string& projectionID);

    // Search
    Projection* findProjection(
        const string& id);

    // Deletes a Projection outright - the plan is scrapped, not the
    // Procurement Orders already placed from it (those are real
    // commitments and stay; they simply stop being linked to a
    // Projection, the same as any order placed with no Projection at
    // all). Refuses once the Projection is completed - a produced,
    // archived batch is a historical record and stays.
    bool deleteProjection(
        const string& id);

    // Confirms production: checks every BOM material has enough
    // actual stock for producedQuantity units, issues (consumes) it
    // all from the Projection's Warehouse, then marks the
    // Projection completed - it moves to Archived Projections and
    // stops reserving virtual stock for anyone else. Only allowed
    // once (a Projection already completed is refused), and nothing
    // is issued at all if any material would fall short.
    bool completeProjection(
        const string& id,
        int producedQuantity,
        const string& completionDate);

    // Access
    const vector<unique_ptr<Projection>>&
        getProjections() const;

    // Persistence
    bool save();
    bool load();

    // Clear all projections (used when reloading system data)
    void clear();
};
