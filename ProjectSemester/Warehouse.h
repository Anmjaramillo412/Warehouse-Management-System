#pragma once

#include "Material.h"

using namespace std;


// ================================================================
// NODE
// ================================================================

struct WarehouseNode
{
    Material* material;

    int quantity;

    WarehouseNode* previous;

    WarehouseNode* next;
};


// ================================================================
// WAREHOUSE
// ================================================================

class Warehouse
{
private:

    int ID;

    string name;

    WarehouseNode* head;

    WarehouseNode* tail;

public:

    // Constructor
    Warehouse(
        int id = 0,
        string n = "");

    // Destructor
    ~Warehouse();

    // Getters
    int getID() const;

    string getName() const;

    // Setters
    void setID(int id);

    void setName(string n);

    // Material operations
    bool addMaterial(
        Material* material,
        int quantity);

    bool removeMaterial(
        const string& materialID,
        int quantity);

    // Search
    WarehouseNode* findMaterial(
        const string& materialID) const;

    // Status
    bool isEmpty() const;

    // Inventory access
    WarehouseNode* getHead() const;

    // Display
    void display() const;
};