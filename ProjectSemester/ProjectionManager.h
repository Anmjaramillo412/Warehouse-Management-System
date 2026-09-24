#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Projection.h"
#include "ProductManager.h"
#include "WarehouseManager.h"
#include "InventoryManager.h"
#include "MovementLogger.h"

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

    // Generates the next consecutive ID, e.g. "PRJ-000001"
    string generateNextID();

    // Stock of a material summed across every warehouse
    int getTotalStock(
        const string& materialID) const;

public:

    // "Virtual" stock: total warehouse stock for this material minus
    // what every other still-active Projection (deadline today or
    // later, or no deadline at all) has already reserved for it.
    // This is what a Projection must check to decide whether it is
    // actually short on a material - otherwise two Projections could
    // both plan around the same physical units of stock. Pass the
    // Projection's own ID as excludeProjectionID so a Projection
    // does not subtract its own reservation from itself.
    int getVirtualStock(
        const string& materialID,
        const string& excludeProjectionID = "") const;

    // Constructor
    ProjectionManager(
        ProductManager* prodManager = nullptr,
        WarehouseManager* whManager = nullptr,
        InventoryManager* invManager = nullptr,
        MovementLogger* logger = nullptr,
        string file = "data/projections.txt");

    void setProductManager(
        ProductManager* manager);

    void setWarehouseManager(
        WarehouseManager* manager);

    void setInventoryManager(
        InventoryManager* manager);

    void setMovementLogger(
        MovementLogger* logger);

    // Creates a new Projection from the Product's BOM, the quantity
    // to manufacture, and current stock (only materials with a
    // shortfall are included). Returns nullptr if the Product does
    // not exist, has no BOM, or nothing is actually short.
    Projection* createProjection(
        const string& productID,
        int warehouseID,
        const string& deadline,
        int manufactureQuantity,
        const string& creationDate);

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
