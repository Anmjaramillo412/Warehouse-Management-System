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
