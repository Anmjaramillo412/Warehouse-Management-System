#include "SupplierManager.h"

#include <iostream>

using namespace std;


// ================================================================
// CREATE SUPPLIER
// ================================================================

bool SupplierManager::createSupplier(
    const Supplier& supplier)
{
    // Name must not be empty

    if (supplier.getName().empty())
    {
        return false;
    }


    // Check if a supplier with this name already exists

    if (findSupplier(
        supplier.getName()) != nullptr)
    {
        return false;
    }


    // Create independent Supplier object

    suppliers.push_back(
        make_unique<Supplier>(supplier));


    return true;
}


// ================================================================
// FIND SUPPLIER
// ================================================================

Supplier* SupplierManager::findSupplier(
    const string& name)
{
    for (const auto& supplier : suppliers)
    {
        if (supplier->getName() == name)
        {
            return supplier.get();
        }
    }

    return nullptr;
}


// ================================================================
// MODIFY SUPPLIER
// ================================================================

bool SupplierManager::modifySupplier(
    const string& name,
    const Supplier& newData)
{
    Supplier* supplier =
        findSupplier(name);


    if (supplier == nullptr)
    {
        return false;
    }


    // Do not modify the name (unique key) here.

    supplier->setAddress(
        newData.getAddress());

    supplier->setCountry(
        newData.getCountry());

    supplier->setContactName(
        newData.getContactName());

    supplier->setContactEmail(
        newData.getContactEmail());

    supplier->setWebsite(
        newData.getWebsite());

    supplier->setOrderingMethods(
        newData.getOrderingMethods());

    supplier->setPaymentMethod(
        newData.getPaymentMethod());

    supplier->setLeadTimeWeeks(
        newData.getLeadTimeWeeks());


    return true;
}


// ================================================================
// DELETE SUPPLIER
// ================================================================

bool SupplierManager::deleteSupplier(
    const string& name)
{
    for (auto it = suppliers.begin();
        it != suppliers.end();
        ++it)
    {
        if ((*it)->getName() == name)
        {
            suppliers.erase(it);

            return true;
        }
    }

    return false;
}


// ================================================================
// DISPLAY SUPPLIERS
// ================================================================

void SupplierManager::displaySuppliers() const
{
    if (suppliers.empty())
    {
        cout << endl;

        cout << "No suppliers available."
            << endl;

        return;
    }


    cout << endl;

    cout << "========== SUPPLIERS =========="
        << endl;


    for (const auto& supplier : suppliers)
    {
        supplier->display();

        cout << endl;
    }
}


// ================================================================
// GET SUPPLIERS
// ================================================================

const vector<unique_ptr<Supplier>>&
SupplierManager::getSuppliers() const
{
    return suppliers;
}


// ================================================================
// CLEAR
// ================================================================

void SupplierManager::clear()
{
    suppliers.clear();
}
