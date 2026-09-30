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

    // Sum of this material's own requiredQuantity across every
    // currently open Projection (not completed, deadline today or
    // later, or no deadline at all) tied to this ONE Warehouse. This
    // is the "everyone who is competing for this material in this
    // Warehouse" total - see getQuantityToOrder() below for why it is
    // never split/attributed per-Projection any more.
    int getOpenRequiredQuantity(
        const string& materialID,
        int warehouseID) const;

public:

    // Actual current stock of one material inside one Warehouse right
    // now - no Projection reservations subtracted (see
    // getQuantityToOrder() for that).
    int getWarehouseStock(
        const string& materialID,
        int warehouseID) const;

    // Live, SHARED "Qty to Order" for one material inside one
    // Warehouse: the combined requirement of every currently open
    // Projection tied to that Warehouse that needs this material
    // (this necessarily includes the Projection asking, once it
    // exists, since it is part of "every open Projection") minus that
    // Warehouse's actual current stock, floored at 0.
    //
    // This replaces the old per-Projection "Virtual Stock" model
    // (total stock minus every OTHER open Projection's reservation,
    // computed independently for each Projection), which double-
    // counted a scarce material whenever two or more Projections
    // competed for it: each one excluded only itself and so each
    // concluded, on its own, that it needed to cover the full
    // combined shortfall a second time. Because this number is now
    // computed once from the shared totals instead of per-Projection,
    // it comes out identical (and correct) no matter which open
    // Projection asks for it.
    int getQuantityToOrder(
        const string& materialID,
        int warehouseID) const;

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
