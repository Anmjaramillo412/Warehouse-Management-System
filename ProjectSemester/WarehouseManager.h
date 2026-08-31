#pragma once

#include <vector>
#include <string>

#include "Warehouse.h"

using namespace std;

class WarehouseManager
{
private:

    vector<Warehouse*> warehouses;

public:

    // Constructor
    WarehouseManager();

    // Destructor
    ~WarehouseManager();

    // Create
    bool createWarehouse(int id, string name);

    // Search
    Warehouse* findWarehouse(int id);

    // Delete
    bool deleteWarehouse(int id);

    // Display
    void displayWarehouses() const;

    // Material operations
    bool addMaterialToWarehouse(
        int warehouseID,
        Material* material,
        int quantity);

    bool removeMaterialFromWarehouse(
        int warehouseID,
        const string& materialID,
        int quantity);

    bool isMaterialInUse(
        const string& materialID) const;

    // Access
    vector<Warehouse*>& getWarehouses();
};