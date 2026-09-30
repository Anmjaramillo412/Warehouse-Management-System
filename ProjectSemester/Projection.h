#pragma once

#include <string>
#include <vector>

using namespace std;


// ================================================================
// PROJECTION ITEM
// ================================================================
// One material line inside a Production Projection: how much of
// this material is required to manufacture the projected quantity,
// and how much stock existed when the projection was created. Both
// numbers are frozen at creation time (a "snapshot"), so a
// projection's plan does not shift if stock changes later.

struct ProjectionItem
{
    string materialID;

    int requiredQuantity;

    int stockAtCreation;

    // How many times a Procurement Order has been registered against
    // this line FROM this Projection (see Projection::
    // registerOrderAttempt()). Capped in practice at 2 - see
    // orderRegistered below.
    int orderRegistrationCount = 0;

    // Frozen "no more ordering from this line" flag. Set the moment
    // either: (a) a single registration already covered this line's
    // full requiredQuantity, or (b) two registrations have been made
    // from this line regardless of whether they were enough - by
    // design, a line only ever gets at most two chances to be ordered
    // from the Projection view; anything still missing after that is
    // placed manually from New Order instead. Once set it does not
    // un-set itself if stock or orders change later (e.g. an order
    // gets cancelled) - it stays closed until the whole Projection is
    // completed.
    bool orderRegistered = false;
};


// ================================================================
// PROJECTION
// ================================================================
// One Production Projection batch: "to manufacture X units of
// Product P by date D, these materials and quantities are needed."
// Procurement Orders placed from this batch reference it internally
// (see ProcurementOrder::getProjectionID()), so "how much of this
// batch has already been ordered" can be computed even after
// navigating away and back, and even if the same Product is
// projected more than once.

class Projection
{
private:

    string id;

    string productID;

    int warehouseID;

    string deadline;

    int manufactureQuantity;

    string creationDate;

    vector<ProjectionItem> items;

    // Set once production is confirmed (see completeProjection() in
    // ProjectionManager): the BOM materials have been issued from
    // the Warehouse and this batch moves to Archived Projections.
    bool completed;

    int producedQuantity;

    string completionDate;

public:

    // Constructor
    Projection(
        string projectionID = "",
        string prodID = "",
        int whID = 0,
        string dl = "",
        int mfgQuantity = 0,
        string cDate = "");

    // Destructor
    ~Projection();

    // Getters
    string getID() const;
    string getProductID() const;
    int getWarehouseID() const;
    string getDeadline() const;
    int getManufactureQuantity() const;
    string getCreationDate() const;
    const vector<ProjectionItem>& getItems() const;

    // Setters
    void setID(const string& projectionID);
    void setProductID(const string& prodID);
    void setWarehouseID(int whID);
    void setDeadline(const string& dl);
    void setManufactureQuantity(int mfgQuantity);
    void setCreationDate(const string& cDate);

    // Items
    void addItem(
        const string& materialID,
        int requiredQuantity,
        int stockAtCreation);

    void setItems(
        const vector<ProjectionItem>& newItems);

    // Registers one order-placement attempt against this item from
    // the Projection UI: increments orderRegistrationCount, and
    // freezes orderRegistered (see ProjectionItem) if fullyCovered is
    // true or this is now the item's second attempt. No-op if
    // materialID is not one of this Projection's items.
    void registerOrderAttempt(
        const string& materialID,
        bool fullyCovered);

    // Restores a persisted item's order-registration state exactly as
    // saved - used only by ProjectionManager::load(). No-op if
    // materialID is not one of this Projection's items.
    void setItemOrderState(
        const string& materialID,
        bool registered,
        int registrationCount);

    // Completion (production confirmed / archived)
    bool isCompleted() const;
    int getProducedQuantity() const;
    string getCompletionDate() const;

    void setCompleted(
        bool value,
        int produced = 0,
        const string& cDate = "");

    // Display
    void display() const;
};
