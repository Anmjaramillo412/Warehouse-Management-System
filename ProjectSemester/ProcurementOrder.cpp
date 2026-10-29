#include "ProcurementOrder.h"

#include <iostream>
#include <algorithm>


// ================================================================
// CONSTRUCTOR
// ================================================================

ProcurementOrder::ProcurementOrder(
    string orderID,
    string prodID,
    string matID,
    int whID,
    string oDate,
    double oQuantity,
    string cmt)
{
    id = orderID;
    productID = prodID;
    materialID = matID;
    materialName = "";
    warehouseID = whID;
    orderDate = oDate;
    orderedQuantity = roundQty(oQuantity);
    comment = cmt;

    confirmationDate = "";
    confirmedQuantity = 0;
    projectionID = "";
    closed = false;
    cancelled = false;
    estimatedReceiptDate = "";
}


// ================================================================
// DESTRUCTOR
// ================================================================

ProcurementOrder::~ProcurementOrder()
{
}


// ================================================================
// GETTERS
// ================================================================

string ProcurementOrder::getID() const
{
    return id;
}


string ProcurementOrder::getProductID() const
{
    return productID;
}


string ProcurementOrder::getMaterialID() const
{
    return materialID;
}


string ProcurementOrder::getMaterialName() const
{
    return materialName;
}


int ProcurementOrder::getWarehouseID() const
{
    return warehouseID;
}


string ProcurementOrder::getOrderDate() const
{
    return orderDate;
}


double ProcurementOrder::getOrderedQuantity() const
{
    return orderedQuantity;
}


string ProcurementOrder::getComment() const
{
    return comment;
}


string ProcurementOrder::getConfirmationDate() const
{
    return confirmationDate;
}


double ProcurementOrder::getConfirmedQuantity() const
{
    return confirmedQuantity;
}


const vector<ProcurementReceipt>& ProcurementOrder::getReceipts() const
{
    return receipts;
}


string ProcurementOrder::getProjectionID() const
{
    return projectionID;
}


bool ProcurementOrder::isClosed() const
{
    return closed;
}


bool ProcurementOrder::isCancelled() const
{
    return cancelled;
}


string ProcurementOrder::getEstimatedReceiptDate() const
{
    return estimatedReceiptDate;
}


// ================================================================
// SETTERS
// ================================================================

void ProcurementOrder::setID(const string& orderID)
{
    id = orderID;
}


void ProcurementOrder::setProductID(const string& prodID)
{
    productID = prodID;
}


void ProcurementOrder::setMaterialID(const string& matID)
{
    materialID = matID;
}


void ProcurementOrder::setMaterialName(const string& matName)
{
    materialName = matName;
}


void ProcurementOrder::setWarehouseID(int whID)
{
    warehouseID = whID;
}


void ProcurementOrder::setOrderDate(const string& oDate)
{
    orderDate = oDate;
}


void ProcurementOrder::setOrderedQuantity(double oQuantity)
{
    orderedQuantity = roundQty(oQuantity);
}


void ProcurementOrder::setComment(const string& cmt)
{
    comment = cmt;
}


void ProcurementOrder::setConfirmationDate(const string& cDate)
{
    confirmationDate = cDate;
}


void ProcurementOrder::setConfirmedQuantity(double cQuantity)
{
    confirmedQuantity = roundQty(cQuantity);
}


void ProcurementOrder::setProjectionID(const string& projID)
{
    projectionID = projID;
}


void ProcurementOrder::setClosed(bool value)
{
    closed = value;
}


void ProcurementOrder::setCancelled(bool value)
{
    cancelled = value;
}


void ProcurementOrder::setEstimatedReceiptDate(const string& date)
{
    estimatedReceiptDate = date;
}


// ================================================================
// RECEIPTS
// ================================================================

void ProcurementOrder::addReceipt(
    const string& receiptDate,
    double receivedQuantity,
    const string& cmt)
{
    ProcurementReceipt receipt;

    receipt.receiptDate = receiptDate;
    receipt.receivedQuantity = roundQty(receivedQuantity);
    receipt.comment = cmt;

    receipts.push_back(receipt);
}


void ProcurementOrder::setReceipts(
    const vector<ProcurementReceipt>& newReceipts)
{
    receipts = newReceipts;
}


// ================================================================
// COMPUTED - TOTAL RECEIVED QUANTITY
// ================================================================

double ProcurementOrder::getTotalReceivedQuantity() const
{
    double total = 0.0;

    for (const auto& receipt : receipts)
    {
        total = roundQty(total + receipt.receivedQuantity);
    }

    return total;
}


// ================================================================
// COMPUTED - TARGET QUANTITY
// ================================================================

double ProcurementOrder::getTargetQuantity() const
{
    if (confirmedQuantity > 0)
    {
        return confirmedQuantity;
    }

    return orderedQuantity;
}


// ================================================================
// COMPUTED - PENDING QUANTITY
// ================================================================

double ProcurementOrder::getPendingQuantity() const
{
    double pending =
        roundQty(getTargetQuantity() - getTotalReceivedQuantity());

    return (pending > 0) ? pending : 0.0;
}


// ================================================================
// COMPUTED - STATUS
// ================================================================

string ProcurementOrder::getStatus() const
{
    if (cancelled)
    {
        return "Cancelled";
    }

    if (closed)
    {
        return "Closed (Incomplete)";
    }

    if (getTotalReceivedQuantity() > 0 &&
        getPendingQuantity() <= 0)
    {
        return "Completed";
    }

    if (getTotalReceivedQuantity() > 0)
    {
        return "Partially Received";
    }

    if (!confirmationDate.empty())
    {
        return "Confirmed";
    }

    return "Ordered";
}


// ================================================================
// DISPLAY
// ================================================================

void ProcurementOrder::display() const
{
    cout << endl;

    cout << "----------------------------------------"
        << endl;

    cout << "ID:               " << id << endl;
    cout << "Product:          " << productID << endl;
    cout << "Material:         " << materialID << endl;
    cout << "Warehouse:        " << warehouseID << endl;
    cout << "Order Date:       " << orderDate << endl;
    cout << "Ordered Qty:      " << formatQty(orderedQuantity) << endl;
    cout << "Confirmation:     " << confirmationDate << endl;
    cout << "Confirmed Qty:    " << formatQty(confirmedQuantity) << endl;
    cout << "Received Qty:     " << formatQty(getTotalReceivedQuantity()) << endl;
    cout << "Status:           " << getStatus() << endl;
    cout << "Comment:          " << comment << endl;

    cout << "----------------------------------------"
        << endl;
}
