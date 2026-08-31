#include "Product.h"

#include <iostream>

using namespace std;


// ================================================================
// CONSTRUCTOR
// ================================================================

Product::Product(
    string id,
    string n,
    string d)
{
    ID = id;

    name = n;

    description = d;
}


// ================================================================
// DESTRUCTOR
// ================================================================

Product::~Product()
{
}


// ================================================================
// GETTERS
// ================================================================

string Product::getID() const
{
    return ID;
}


string Product::getName() const
{
    return name;
}


string Product::getDescription() const
{
    return description;
}


const vector<BOMItem>&
Product::getBOM() const
{
    return bom;
}


// ================================================================
// SETTERS
// ================================================================

void Product::setID(
    const string& id)
{
    ID = id;
}


void Product::setName(
    const string& n)
{
    name = n;
}


void Product::setDescription(
    const string& d)
{
    description = d;
}


// ================================================================
// ADD BOM ITEM
// ================================================================

bool Product::addBOMItem(
    const string& materialID,
    int quantity)
{
    if (materialID.empty())
    {
        return false;
    }


    if (quantity <= 0)
    {
        return false;
    }


    // Check if component already exists

    BOMItem* existing =
        findBOMItem(materialID);


    if (existing != nullptr)
    {
        existing->quantity += quantity;

        return true;
    }


    BOMItem item;

    item.materialID = materialID;

    item.quantity = quantity;


    bom.push_back(item);


    return true;
}


// ================================================================
// REMOVE BOM ITEM
// ================================================================

bool Product::removeBOMItem(
    const string& materialID)
{
    for (auto it = bom.begin();
        it != bom.end();
        ++it)
    {
        if (it->materialID == materialID)
        {
            bom.erase(it);

            return true;
        }
    }


    return false;
}


// ================================================================
// FIND BOM ITEM
// ================================================================

BOMItem* Product::findBOMItem(
    const string& materialID)
{
    for (auto& item : bom)
    {
        if (item.materialID == materialID)
        {
            return &item;
        }
    }


    return nullptr;
}


// ================================================================
// DISPLAY
// ================================================================

void Product::display() const
{
    cout << endl;

    cout << "========================================"
        << endl;

    cout << "Product ID: "
        << ID
        << endl;

    cout << "Product Name: "
        << name
        << endl;

    cout << "Description: "
        << description
        << endl;

    cout << "BOM:"
        << endl;


    if (bom.empty())
    {
        cout << "  No components."
            << endl;
    }
    else
    {
        for (const auto& item : bom)
        {
            cout << "  Material: "
                << item.materialID
                << " | Quantity: "
                << item.quantity
                << endl;
        }
    }


    cout << "========================================"
        << endl;
}