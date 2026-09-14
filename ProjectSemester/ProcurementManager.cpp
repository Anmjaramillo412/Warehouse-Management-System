#include "ProcurementManager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <algorithm>

using namespace std;

namespace fs = std::filesystem;


// ================================================================
// FIELD SANITIZING (free text fields must not break the format)
// ================================================================

static string sanitizeField(const string& value)
{
    string result = value;

    replace(result.begin(), result.end(), '|', ' ');
    replace(result.begin(), result.end(), '~', ' ');
    replace(result.begin(), result.end(), ',', ';');
    replace(result.begin(), result.end(), '\n', ' ');
    replace(result.begin(), result.end(), '\r', ' ');

    return result;
}


// ================================================================
// SPLIT HELPER
// ================================================================

static vector<string> splitBy(
    const string& value,
    char delimiter)
{
    vector<string> result;

    stringstream stream(value);

    string token;

    while (getline(stream, token, delimiter))
    {
        result.push_back(token);
    }

    return result;
}


// ================================================================
// CONSTRUCTOR
// ================================================================

ProcurementManager::ProcurementManager(
    MaterialManager* matManager,
    InventoryManager* invManager,
    string file)
{
    materialManager = matManager;
    inventoryManager = invManager;

    filename = file;

    nextNumber = 1;

    // Create parent directory if necessary

    fs::path path(filename);

    if (!path.parent_path().empty())
    {
        fs::create_directories(
            path.parent_path());
    }

    load();
}


// ================================================================
// SET MATERIAL MANAGER
// ================================================================

void ProcurementManager::setMaterialManager(
    MaterialManager* manager)
{
    materialManager = manager;
}


// ================================================================
// SET INVENTORY MANAGER
// ================================================================

void ProcurementManager::setInventoryManager(
    InventoryManager* manager)
{
    inventoryManager = manager;
}


// ================================================================
// GENERATE NEXT ID
// ================================================================

string ProcurementManager::generateNextID()
{
    ostringstream stream;

    stream << "PRC-"
        << setfill('0')
        << setw(6)
        << nextNumber;

    nextNumber++;

    return stream.str();
}


// ================================================================
// CREATE ORDER
// ================================================================

ProcurementOrder* ProcurementManager::createOrder(
    const string& productID,
    const string& materialID,
    int warehouseID,
    const string& orderDate,
    int orderedQuantity,
    const string& comment)
{
    if (materialID.empty() ||
        orderedQuantity <= 0)
    {
        return nullptr;
    }

    string id =
        generateNextID();

    orders.push_back(
        make_unique<ProcurementOrder>(
            id,
            productID,
            materialID,
            warehouseID,
            orderDate,
            orderedQuantity,
            sanitizeField(comment)));

    save();

    return orders.back().get();
}


// ================================================================
// FIND ORDER
// ================================================================

ProcurementOrder* ProcurementManager::findOrder(
    const string& id)
{
    for (const auto& order : orders)
    {
        if (order->getID() == id)
        {
            return order.get();
        }
    }

    return nullptr;
}


// ================================================================
// CONFIRM ORDER
// ================================================================

bool ProcurementManager::confirmOrder(
    const string& id,
    const string& confirmationDate,
    int confirmedQuantity)
{
    ProcurementOrder* order =
        findOrder(id);

    if (order == nullptr)
    {
        return false;
    }

    if (confirmedQuantity <= 0)
    {
        return false;
    }

    order->setConfirmationDate(
        confirmationDate);

    order->setConfirmedQuantity(
        confirmedQuantity);

    save();

    return true;
}


// ================================================================
// RECEIVE ORDER
// ================================================================
// Adds a (possibly partial) receipt to the order and performs the
// matching Goods Receipt into the order's Warehouse, the same way
// the Goods Receipt module does.

bool ProcurementManager::receiveOrder(
    const string& id,
    const string& receiptDate,
    int receivedQuantity,
    const string& comment)
{
    ProcurementOrder* order =
        findOrder(id);

    if (order == nullptr)
    {
        return false;
    }

    if (receivedQuantity <= 0)
    {
        return false;
    }

    if (materialManager == nullptr ||
        inventoryManager == nullptr)
    {
        return false;
    }

    Material* material =
        materialManager->findMaterial(
            order->getMaterialID());

    if (material == nullptr)
    {
        return false;
    }

    bool receiptSuccess =
        inventoryManager->goodsReceipt(
            order->getWarehouseID(),
            material,
            receivedQuantity,
            "Procurement " + id +
                (comment.empty() ? "" : (" - " + comment)));

    if (!receiptSuccess)
    {
        return false;
    }

    order->addReceipt(
        receiptDate,
        receivedQuantity,
        sanitizeField(comment));

    save();

    return true;
}


// ================================================================
// GET ORDERS
// ================================================================

const vector<unique_ptr<ProcurementOrder>>&
ProcurementManager::getOrders() const
{
    return orders;
}


// ================================================================
// SAVE
// ================================================================
// Plain text format, one line per order:
//
// id|productID|materialID|warehouseID|orderDate|orderedQuantity|
//   comment|confirmationDate|confirmedQuantity|receipts
//
// receipts: each as "date,quantity,comment", several joined by "~"

bool ProcurementManager::save()
{
    ofstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    for (const auto& order : orders)
    {
        string receiptsField = "";

        const auto& receipts =
            order->getReceipts();

        for (size_t i = 0; i < receipts.size(); i++)
        {
            if (i > 0)
            {
                receiptsField += "~";
            }

            receiptsField +=
                receipts[i].receiptDate + "," +
                to_string(receipts[i].receivedQuantity) + "," +
                receipts[i].comment;
        }

        file << order->getID() << "|"
            << order->getProductID() << "|"
            << order->getMaterialID() << "|"
            << order->getWarehouseID() << "|"
            << order->getOrderDate() << "|"
            << order->getOrderedQuantity() << "|"
            << order->getComment() << "|"
            << order->getConfirmationDate() << "|"
            << order->getConfirmedQuantity() << "|"
            << receiptsField
            << endl;
    }

    file.close();

    return true;
}


// ================================================================
// LOAD
// ================================================================

bool ProcurementManager::load()
{
    ifstream file(filename);

    if (!file.is_open())
    {
        // No file yet - not an error, just nothing to load.
        return true;
    }

    orders.clear();

    int highestNumber = 0;

    string line;

    while (getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        vector<string> fields =
            splitBy(line, '|');

        if (fields.size() < 9)
        {
            continue;
        }

        auto order = make_unique<ProcurementOrder>(
            fields[0],
            fields[1],
            fields[2],
            atoi(fields[3].c_str()),
            fields[4],
            atoi(fields[5].c_str()),
            fields[6]);

        order->setConfirmationDate(
            fields[7]);

        order->setConfirmedQuantity(
            atoi(fields[8].c_str()));

        if (fields.size() > 9 &&
            !fields[9].empty())
        {
            vector<string> receiptTokens =
                splitBy(fields[9], '~');

            for (const string& token : receiptTokens)
            {
                vector<string> receiptFields =
                    splitBy(token, ',');

                if (receiptFields.size() < 2)
                {
                    continue;
                }

                string receiptComment =
                    receiptFields.size() > 2 ?
                    receiptFields[2] : "";

                order->addReceipt(
                    receiptFields[0],
                    atoi(receiptFields[1].c_str()),
                    receiptComment);
            }
        }

        // Track the highest existing "PRC-######" number so new
        // orders keep counting up instead of reusing IDs.

        string idNumberPart =
            fields[0].size() > 4 ?
            fields[0].substr(4) : "";

        int idNumber =
            atoi(idNumberPart.c_str());

        if (idNumber > highestNumber)
        {
            highestNumber = idNumber;
        }

        orders.push_back(
            std::move(order));
    }

    file.close();

    nextNumber = highestNumber + 1;

    return true;
}


// ================================================================
// CLEAR
// ================================================================

void ProcurementManager::clear()
{
    orders.clear();

    nextNumber = 1;
}
