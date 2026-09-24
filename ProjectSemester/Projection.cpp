#include "Projection.h"

#include <iostream>


// ================================================================
// CONSTRUCTOR
// ================================================================

Projection::Projection(
    string projectionID,
    string prodID,
    int whID,
    string dl,
    int mfgQuantity,
    string cDate)
{
    id = projectionID;
    productID = prodID;
    warehouseID = whID;
    deadline = dl;
    manufactureQuantity = mfgQuantity;
    creationDate = cDate;

    completed = false;
    producedQuantity = 0;
    completionDate = "";
}


// ================================================================
// DESTRUCTOR
// ================================================================

Projection::~Projection()
{
}


// ================================================================
// GETTERS
// ================================================================

string Projection::getID() const
{
    return id;
}


string Projection::getProductID() const
{
    return productID;
}


int Projection::getWarehouseID() const
{
    return warehouseID;
}


string Projection::getDeadline() const
{
    return deadline;
}


int Projection::getManufactureQuantity() const
{
    return manufactureQuantity;
}


string Projection::getCreationDate() const
{
    return creationDate;
}


const vector<ProjectionItem>& Projection::getItems() const
{
    return items;
}


// ================================================================
// SETTERS
// ================================================================

void Projection::setID(const string& projectionID)
{
    id = projectionID;
}


void Projection::setProductID(const string& prodID)
{
    productID = prodID;
}


void Projection::setWarehouseID(int whID)
{
    warehouseID = whID;
}


void Projection::setDeadline(const string& dl)
{
    deadline = dl;
}


void Projection::setManufactureQuantity(int mfgQuantity)
{
    manufactureQuantity = mfgQuantity;
}


void Projection::setCreationDate(const string& cDate)
{
    creationDate = cDate;
}


// ================================================================
// ITEMS
// ================================================================

void Projection::addItem(
    const string& materialID,
    int requiredQuantity,
    int stockAtCreation)
{
    ProjectionItem item;

    item.materialID = materialID;
    item.requiredQuantity = requiredQuantity;
    item.stockAtCreation = stockAtCreation;

    items.push_back(item);
}


void Projection::setItems(
    const vector<ProjectionItem>& newItems)
{
    items = newItems;
}


// ================================================================
// COMPLETION (production confirmed / archived)
// ================================================================

bool Projection::isCompleted() const
{
    return completed;
}


int Projection::getProducedQuantity() const
{
    return producedQuantity;
}


string Projection::getCompletionDate() const
{
    return completionDate;
}


void Projection::setCompleted(
    bool value,
    int produced,
    const string& cDate)
{
    completed = value;
    producedQuantity = produced;
    completionDate = cDate;
}


// ================================================================
// DISPLAY
// ================================================================

void Projection::display() const
{
    cout << endl;

    cout << "----------------------------------------"
        << endl;

    cout << "ID:               " << id << endl;
    cout << "Product:          " << productID << endl;
    cout << "Warehouse:        " << warehouseID << endl;
    cout << "Deadline:         " << deadline << endl;
    cout << "Manufacture Qty:  " << manufactureQuantity << endl;
    cout << "Created:          " << creationDate << endl;
    cout << "Items:            " << items.size() << endl;

    cout << "----------------------------------------"
        << endl;
}
