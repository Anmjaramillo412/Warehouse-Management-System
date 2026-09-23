#pragma once

#include <string>
#include <vector>

using namespace std;


// ================================================================
// BOM ITEM
// ================================================================

struct BOMItem
{
    string materialID;

    int quantity;
};


// ================================================================
// PRODUCT
// ================================================================

class Product
{
private:

    string ID;

    string name;

    string description;

    vector<BOMItem> bom;

public:

    // Constructor
    Product(
        string id = "",
        string n = "",
        string d = "");

    // Destructor
    ~Product();

    // Getters
    string getID() const;

    string getName() const;

    string getDescription() const;

    const vector<BOMItem>& getBOM() const;

    // Setters
    void setID(const string& id);

    void setName(const string& n);

    void setDescription(const string& d);

    // BOM operations
    bool addBOMItem(
        const string& materialID,
        int quantity);

    bool removeBOMItem(
        const string& materialID);

    BOMItem* findBOMItem(
        const string& materialID);

    // Wholesale replace, used by ProductManager::modifyProduct() -
    // unlike addBOMItem() (which merges into an existing line),
    // this is a straight swap so a Modify submission's BOM always
    // ends up exactly as sent, with no leftover pre-edit lines.
    void setBOM(
        const vector<BOMItem>& newBom);

    // Display
    void display() const;
};