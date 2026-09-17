#pragma once

#include <string>
#include <vector>

using namespace std;


// ================================================================
// PROCUREMENT RECEIPT
// ================================================================
// One partial (or full) delivery received against a Procurement
// Order. Adding one immediately triggers the matching Goods
// Receipt into the order's Warehouse (see ProcurementManager).

struct ProcurementReceipt
{
    string receiptDate;

    int receivedQuantity;

    string comment;
};


// ================================================================
// PROCUREMENT ORDER
// ================================================================
// One Procurement Order line: a single Material, ordered for one
// Warehouse, optionally traced back to the Product/projection that
// generated it. Confirmation is a single step; receiving can
// happen across several partial deliveries (see ProcurementReceipt).

class ProcurementOrder
{
private:

    string id;

    string productID;
    string materialID;

    // Snapshot of the Material's name at order time, saved right
    // alongside materialID so the persisted record stays readable on
    // its own (e.g. opening data/procurement_orders.txt directly)
    // even if the Material is later renamed or removed.
    string materialName;

    int warehouseID;

    string orderDate;
    int orderedQuantity;

    string comment;

    string confirmationDate;
    int confirmedQuantity;

    vector<ProcurementReceipt> receipts;

    // Internal only (not shown in the UI): which Projection batch,
    // if any, this order was registered from. Lets a Projection
    // compute "how much of this batch have I already ordered" even
    // if the same Product was projected more than once. Product ID
    // above is unaffected and remains the user-facing link.
    string projectionID;

    // Set when the supplier will never deliver the rest (goods lost,
    // discontinued, order cancelled after partial delivery, etc.).
    // Whatever was actually received stays in inventory; the line
    // just stops counting as pending and moves to Archived Orders
    // instead of sitting in Confirmed Orders forever waiting for
    // units that are not coming.
    bool closed;

    // Set when this order is cancelled while still unconfirmed (the
    // supplier never got the chance to confirm it, and it never
    // will). Only applies to lines that were never confirmed - a
    // confirmed order is closed instead, never cancelled. Moves the
    // order into Archived Orders labeled "Cancelled".
    bool cancelled;

public:

    // Constructor
    ProcurementOrder(
        string orderID = "",
        string prodID = "",
        string matID = "",
        int whID = 0,
        string oDate = "",
        int oQuantity = 0,
        string cmt = "");

    // Destructor
    ~ProcurementOrder();

    // Getters
    string getID() const;
    string getProductID() const;
    string getMaterialID() const;
    string getMaterialName() const;
    int getWarehouseID() const;
    string getOrderDate() const;
    int getOrderedQuantity() const;
    string getComment() const;
    string getConfirmationDate() const;
    int getConfirmedQuantity() const;
    const vector<ProcurementReceipt>& getReceipts() const;
    string getProjectionID() const;
    bool isClosed() const;
    bool isCancelled() const;

    // Setters
    void setID(const string& orderID);
    void setProductID(const string& prodID);
    void setMaterialID(const string& matID);
    void setMaterialName(const string& matName);
    void setWarehouseID(int whID);
    void setOrderDate(const string& oDate);
    void setOrderedQuantity(int oQuantity);
    void setComment(const string& cmt);
    void setConfirmationDate(const string& cDate);
    void setConfirmedQuantity(int cQuantity);
    void setProjectionID(const string& projID);
    void setClosed(bool value);
    void setCancelled(bool value);

    // Receipts
    void addReceipt(
        const string& receiptDate,
        int receivedQuantity,
        const string& cmt = "");

    void setReceipts(
        const vector<ProcurementReceipt>& newReceipts);

    // Computed
    int getTotalReceivedQuantity() const;

    // confirmedQuantity once set (> 0), otherwise orderedQuantity
    int getTargetQuantity() const;

    // max(0, target - received)
    int getPendingQuantity() const;

    // "Ordered" | "Confirmed" | "Partially Received" | "Completed" |
    // "Closed (Incomplete)" | "Cancelled"
    string getStatus() const;

    // Display
    void display() const;
};
