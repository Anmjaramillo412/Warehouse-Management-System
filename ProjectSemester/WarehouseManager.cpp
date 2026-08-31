#include "WarehouseManager.h"

#include <iostream>

using namespace std;


// ================================================================
// CONSTRUCTOR
// ================================================================

WarehouseManager::WarehouseManager()
{
}


// ================================================================
// DESTRUCTOR
// ================================================================

WarehouseManager::~WarehouseManager()
{
    for (Warehouse* warehouse : warehouses)
    {
        delete warehouse;
    }

    warehouses.clear();
}


// ================================================================
// CREATE WAREHOUSE
// ================================================================

bool WarehouseManager::createWarehouse(
    int id,
    string name)
{
    // Check if ID already exists

    if (findWarehouse(id) != nullptr)
    {
        return false;
    }


    // Create warehouse dynamically

    Warehouse* warehouse =
        new Warehouse(id, name);


    // Add pointer to vector

    warehouses.push_back(warehouse);


    return true;
}


// ================================================================
// FIND WAREHOUSE
// ================================================================

Warehouse* WarehouseManager::findWarehouse(int id)
{
    for (Warehouse* warehouse : warehouses)
    {
        if (warehouse->getID() == id)
        {
            return warehouse;
        }
    }

    return nullptr;
}


// ================================================================
// DELETE WAREHOUSE
// ================================================================

bool WarehouseManager::deleteWarehouse(int id)
{
    for (auto it = warehouses.begin();
        it != warehouses.end();
        ++it)
    {
        if ((*it)->getID() == id)
        {
            // Delete the Warehouse object

            delete* it;


            // Remove pointer from vector

            warehouses.erase(it);


            return true;
        }
    }

    return false;
}


// ================================================================
// DISPLAY WAREHOUSES
// ================================================================

void WarehouseManager::displayWarehouses() const
{
    if (warehouses.empty())
    {
        cout << endl;
        cout << "No warehouses available."
            << endl;

        return;
    }


    cout << endl;
    cout << "========== WAREHOUSES =========="
        << endl;


    for (Warehouse* warehouse : warehouses)
    {
        cout << endl;

        cout << "Warehouse ID: "
            << warehouse->getID()
            << endl;

        cout << "Warehouse Name: "
            << warehouse->getName()
            << endl;
    }
}


// ================================================================
// GET WAREHOUSES
// ================================================================

vector<Warehouse*>&
WarehouseManager::getWarehouses()
{
    return warehouses;
}

// ================================================================
// ADD MATERIAL TO WAREHOUSE
// ================================================================

bool WarehouseManager::addMaterialToWarehouse(
    int warehouseID,
    Material* material,
    int quantity)
{
    // Check material pointer
    if (material == nullptr)
    {
        return false;
    }

    // Quantity must be positive
    if (quantity <= 0)
    {
        return false;
    }

    // Find warehouse
    Warehouse* warehouse =
        findWarehouse(warehouseID);

    if (warehouse == nullptr)
    {
        return false;
    }

    // Add material to warehouse
    return warehouse->addMaterial(
        material,
        quantity);
}

bool WarehouseManager::isMaterialInUse(
    const string& materialID) const
{
    for (const Warehouse* warehouse : warehouses)
    {
        if (warehouse->findMaterial(materialID) != nullptr)
        {
            return true;
        }
    }

    return false;
}


// ================================================================
// REMOVE MATERIAL FROM WAREHOUSE
// ================================================================

bool WarehouseManager::removeMaterialFromWarehouse(
    int warehouseID,
    const string& materialID,
    int quantity)
{
    // Quantity must be positive
    if (quantity <= 0)
    {
        return false;
    }

    // Find warehouse
    Warehouse* warehouse =
        findWarehouse(warehouseID);

    if (warehouse == nullptr)
    {
        return false;
    }

    // Remove material from warehouse
    return warehouse->removeMaterial(
        materialID,
        quantity);
}