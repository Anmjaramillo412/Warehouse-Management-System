#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Supplier.h"

using namespace std;

class SupplierManager
{
private:

    vector<unique_ptr<Supplier>> suppliers;

public:

    // Create
    bool createSupplier(
        const Supplier& supplier);

    // Search (name is the unique key)
    Supplier* findSupplier(
        const string& name);

    // Modify
    bool modifySupplier(
        const string& name,
        const Supplier& newData);

    // Delete
    bool deleteSupplier(
        const string& name);

    // Display
    void displaySuppliers() const;

    // Access
    const vector<unique_ptr<Supplier>>&
        getSuppliers() const;

    // Clear all suppliers
    void clear();
};
