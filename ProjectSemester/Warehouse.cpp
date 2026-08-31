#include "Warehouse.h"

#include <iostream>

using namespace std;


// ================================================================
// CONSTRUCTOR
// ================================================================

Warehouse::Warehouse(
    int id,
    string n)
{
    ID = id;

    name = n;

    head = nullptr;

    tail = nullptr;
}


// ================================================================
// DESTRUCTOR
// ================================================================

Warehouse::~Warehouse()
{
    WarehouseNode* current = head;

    while (current != nullptr)
    {
        WarehouseNode* next = current->next;

        delete current;

        current = next;
    }

    head = nullptr;

    tail = nullptr;
}


// ================================================================
// GETTERS
// ================================================================

int Warehouse::getID() const
{
    return ID;
}


string Warehouse::getName() const
{
    return name;
}


// ================================================================
// SETTERS
// ================================================================

void Warehouse::setID(int id)
{
    ID = id;
}


void Warehouse::setName(string n)
{
    name = n;
}


// ================================================================
// FIND MATERIAL
// ================================================================

WarehouseNode* Warehouse::findMaterial(
    const string& materialID) const
{
    WarehouseNode* current = head;

    while (current != nullptr)
    {
        if (current->material->getID() == materialID)
        {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}


// ================================================================
// ADD MATERIAL
// ================================================================

bool Warehouse::addMaterial(
    Material* material,
    int quantity)
{
    if (material == nullptr)
    {
        return false;
    }

    if (quantity <= 0)
    {
        return false;
    }


    // Check if material already exists

    WarehouseNode* existing =
        findMaterial(material->getID());


    if (existing != nullptr)
    {
        existing->quantity += quantity;

        return true;
    }


    // Create new node

    WarehouseNode* newNode =
        new WarehouseNode;


    newNode->material = material;

    newNode->quantity = quantity;

    newNode->previous = tail;

    newNode->next = nullptr;


    // Empty list

    if (head == nullptr)
    {
        head = newNode;

        tail = newNode;
    }
    else
    {
        tail->next = newNode;

        tail = newNode;
    }


    return true;
}


// ================================================================
// REMOVE MATERIAL
// ================================================================

bool Warehouse::removeMaterial(
    const string& materialID,
    int quantity)
{
    if (quantity <= 0)
    {
        return false;
    }


    WarehouseNode* node =
        findMaterial(materialID);


    if (node == nullptr)
    {
        return false;
    }


    // Not enough stock

    if (node->quantity < quantity)
    {
        return false;
    }


    node->quantity -= quantity;


    // If quantity becomes zero,
    // remove the node from the list

    if (node->quantity == 0)
    {
        if (node->previous != nullptr)
        {
            node->previous->next = node->next;
        }
        else
        {
            head = node->next;
        }


        if (node->next != nullptr)
        {
            node->next->previous = node->previous;
        }
        else
        {
            tail = node->previous;
        }


        delete node;
    }


    return true;
}

// ================================================================
// CHECK IF WAREHOUSE IS EMPTY
// ================================================================

bool Warehouse::isEmpty() const
{
    return head == nullptr;
}

// ================================================================
// GET HEAD
// ================================================================

WarehouseNode* Warehouse::getHead() const
{
    return head;
}

// ================================================================
// DISPLAY
// ================================================================

void Warehouse::display() const
{
    cout << endl;

    cout << "========================================"
        << endl;

    cout << "Warehouse ID: "
        << ID
        << endl;

    cout << "Warehouse Name: "
        << name
        << endl;

    cout << "========================================"
        << endl;


    if (head == nullptr)
    {
        cout << "Warehouse is empty."
            << endl;

        return;
    }


    WarehouseNode* current = head;


    while (current != nullptr)
    {
        cout << endl;

        cout << "Material ID: "
            << current->material->getID()
            << endl;

        cout << "Material Name: "
            << current->material->getName()
            << endl;

        cout << "Quantity: "
            << current->quantity
            << " "
            << current->material->getUoM()
            << endl;


        current = current->next;
    }
}