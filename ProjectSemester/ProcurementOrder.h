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

    int warehouseID;

    string orderDate;
    int orderedQuantity;

    string comment;

    string confirmationDate;
    int confirmedQuantity;

    vector<ProcurementReceipt> receipts;

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
    int getWarehouseID() const;
    string getOrderDate() const;
    int getOrderedQuantity() const;
    string getComment() const;
    string getConfirmationDate() const;
    int getConfirmedQuantity() const;
    const vector<ProcurementReceipt>& getReceipts() const;

    // Setters
    void setID(const string& orderID);
    void setProductID(const string& prodID);
    void setMaterialID(const string& matID);
    void setWarehouseID(int whID);
    void setOrderDate(const string& oDate);
    void setOrderedQuantity(int oQuantity);
    void setComment(const string& cmt);
    void setConfirmationDate(const string& cDate);
    void setConfirmedQuantity(int cQuantity);

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

    // "Ordered" | "Confirmed" | "Partially Received" | "Completed"
    string getStatus() const;

    // Display
    void display() const;
};
