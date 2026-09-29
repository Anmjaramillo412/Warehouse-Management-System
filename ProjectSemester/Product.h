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

    // Which Warehouse this product's stock is tracked/displayed
    // against on the Display Products dashboard - most products only
    // really live in one warehouse, and always listing every
    // warehouse's stock (most of it zero/irrelevant) just adds noise.
    // 0 means "none set yet" - Display Products falls back to
    // showing every warehouse in that case, the same as before this
    // field existed. Set from Create Product, or from Modify Product
    // for a product that already exists.
    int mainWarehouseID;

public:

    // Constructor
    Product(
        string id = "",
        string n = "",
        string d = "",
        int mainWhID = 0);

    // Destructor
    ~Product();

    // Getters
    string getID() const;

    string getName() const;

    string getDescription() const;

    const vector<BOMItem>& getBOM() const;

    int getMainWarehouseID() const;

    // Setters
    void setID(const string& id);

    void setName(const string& n);

    void setDescription(const string& d);

    void setMainWarehouseID(int id);

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