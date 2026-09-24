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
// FORMAT-DETECTION HELPER (for load() below)
// ================================================================
// A plain whole number (warehouseID, in the old file layout before
// materialName was added) - vs. free text (materialName, in the
// current layout). Used to tell the two file layouts apart so
// existing data files load correctly without a separate migration
// step.

static bool isAllDigits(const string& value)
{
    if (value.empty())
    {
        return false;
    }

    for (char c : value)
    {
        if (!isdigit(static_cast<unsigned char>(c)))
        {
            return false;
        }
    }

    return true;
}


// ================================================================
// CONSTRUCTOR
// ================================================================

ProcurementManager::ProcurementManager(
    MaterialManager* matManager,
    InventoryManager* invManager,
    MovementLogger* logger,
    string file)
{
    materialManager = matManager;
    inventoryManager = invManager;
    movementLogger = logger;

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


void ProcurementManager::setMovementLogger(
    MovementLogger* logger)
{
    movementLogger = logger;
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
// RESERVE NEXT ID
// ================================================================

string ProcurementManager::reserveNextID()
{
    return generateNextID();
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
    const string& comment,
    const string& projectionID,
    const string& explicitID)
{
    if (materialID.empty() ||
        orderedQuantity <= 0)
    {
        return nullptr;
    }

    string id =
        explicitID.empty() ?
        generateNextID() : explicitID;

    orders.push_back(
        make_unique<ProcurementOrder>(
            id,
            productID,
            materialID,
            warehouseID,
            orderDate,
            orderedQuantity,
            sanitizeField(comment)));

    orders.back()->setProjectionID(
        projectionID);

    // Snapshot the Material's current name into the order itself, so
    // the persisted record stays self-explanatory even if the
    // Material is later renamed or removed from the catalog.
    if (materialManager != nullptr)
    {
        Material* material =
            materialManager->findMaterial(materialID);

        if (material != nullptr)
        {
            orders.back()->setMaterialName(
                material->getName());
        }
    }

    save();

    if (movementLogger != nullptr)
    {
        movementLogger->logSystemEvent(
            "PROCUREMENT ORDER CREATED",
            "ID: " + id +
            " | Product: " + productID +
            " | Material: " + materialID +
            " | Qty: " + to_string(orderedQuantity));
    }

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


ProcurementOrder* ProcurementManager::findOrder(
    const string& id,
    const string& materialID)
{
    for (const auto& order : orders)
    {
        if (order->getID() == id &&
            order->getMaterialID() == materialID)
        {
            return order.get();
        }
    }

    return nullptr;
}


vector<ProcurementOrder*> ProcurementManager::findOrderLines(
    const string& id)
{
    vector<ProcurementOrder*> lines;

    for (const auto& order : orders)
    {
        if (order->getID() == id)
        {
            lines.push_back(order.get());
        }
    }

    return lines;
}


// ================================================================
// CONFIRM ORDER
// ================================================================

bool ProcurementManager::confirmOrder(
    const string& id,
    const string& materialID,
    const string& confirmationDate,
    int confirmedQuantity)
{
    ProcurementOrder* order =
        findOrder(id, materialID);

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

    if (movementLogger != nullptr)
    {
        movementLogger->logSystemEvent(
            "PROCUREMENT ORDER CONFIRMED",
            "ID: " + id +
            " | Material: " + materialID +
            " | Confirmed Qty: " + to_string(confirmedQuantity));
    }

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
    const string& materialID,
    const string& receiptDate,
    int receivedQuantity,
    const string& comment)
{
    ProcurementOrder* order =
        findOrder(id, materialID);

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
// DELETE ORDER
// ================================================================

bool ProcurementManager::deleteOrder(
    const string& id,
    const string& materialID)
{
    for (auto it = orders.begin(); it != orders.end(); ++it)
    {
        if ((*it)->getID() == id &&
            (*it)->getMaterialID() == materialID)
        {
            if ((*it)->getTotalReceivedQuantity() > 0)
            {
                return false;
            }

            // Once a line has been confirmed by the supplier it is
            // part of the confirmed consecutive PRC record and must
            // not disappear - close/cancel it instead of deleting.
            if (!(*it)->getConfirmationDate().empty())
            {
                return false;
            }

            orders.erase(it);

            save();

            if (movementLogger != nullptr)
            {
                movementLogger->logSystemEvent(
                    "PROCUREMENT ORDER DELETED",
                    "ID: " + id + " | Material: " + materialID);
            }

            return true;
        }
    }

    return false;
}


// ================================================================
// CLOSE ORDER (mark still-pending lines as never fully arriving)
// ================================================================

bool ProcurementManager::closeOrder(
    const string& id)
{
    bool closedAny = false;

    for (const auto& order : orders)
    {
        if (order->getID() != id)
        {
            continue;
        }

        if (order->getPendingQuantity() <= 0)
        {
            continue;
        }

        order->setClosed(true);

        closedAny = true;
    }

    if (!closedAny)
    {
        return false;
    }

    save();

    if (movementLogger != nullptr)
    {
        movementLogger->logSystemEvent(
            "PROCUREMENT ORDER CLOSED (INCOMPLETE)",
            "ID: " + id);
    }

    return true;
}


// ================================================================
// CANCEL ORDER (mark every still-unconfirmed line as cancelled)
// ================================================================

bool ProcurementManager::cancelOrder(
    const string& id)
{
    bool cancelledAny = false;

    for (const auto& order : orders)
    {
        if (order->getID() != id)
        {
            continue;
        }

        // Only lines never confirmed by the supplier can be
        // cancelled - a confirmed line is closed instead.
        if (!order->getConfirmationDate().empty())
        {
            continue;
        }

        order->setCancelled(true);

        cancelledAny = true;
    }

    if (!cancelledAny)
    {
        return false;
    }

    save();

    if (movementLogger != nullptr)
    {
        movementLogger->logSystemEvent(
            "PROCUREMENT ORDER CANCELLED",
            "ID: " + id);
    }

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
// id|productID|materialID|materialName|warehouseID|orderDate|
//   orderedQuantity|comment|confirmationDate|confirmedQuantity|
//   receipts|projectionID|closed|cancelled
//
// materialName: snapshot of the Material's name at order time, kept
//   alongside materialID so the file reads on its own even if the
//   Material is later renamed or removed from the catalog. Older
//   rows saved before this field existed have no materialName here -
//   load() tells the two layouts apart (see isAllDigits() above); the
//   name itself is backfilled from the current Material catalog right
//   here in save() (not in load()) because load() runs at program
//   startup, before materials have been imported from the Excel file,
//   so the catalog is not ready yet - save() runs later, after every
//   Procurement action, by which point it is.
// receipts: each as "date,quantity,comment", several joined by "~"
// closed: "1" if closeOrder() was used on this line, else "0"
// cancelled: "1" if cancelOrder() was used on this line, else "0"

bool ProcurementManager::save()
{
    ofstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    // Backfill materialName for any order still missing it (either a
    // pre-materialName row just loaded, or one created before the
    // Material catalog was imported) now that the catalog is likely
    // populated.
    if (materialManager != nullptr)
    {
        for (const auto& order : orders)
        {
            if (!order->getMaterialName().empty())
            {
                continue;
            }

            Material* material =
                materialManager->findMaterial(
                    order->getMaterialID());

            if (material != nullptr)
            {
                order->setMaterialName(
                    material->getName());
            }
        }
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
            << order->getMaterialName() << "|"
            << order->getWarehouseID() << "|"
            << order->getOrderDate() << "|"
            << order->getOrderedQuantity() << "|"
            << order->getComment() << "|"
            << order->getConfirmationDate() << "|"
            << order->getConfirmedQuantity() << "|"
            << receiptsField << "|"
            << order->getProjectionID() << "|"
            << (order->isClosed() ? "1" : "0") << "|"
            << (order->isCancelled() ? "1" : "0")
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

        // Rows saved before materialName existed have warehouseID
        // (a plain number) at index 3; current rows have materialName
        // (free text) there instead, shifting everything after it by
        // one - see isAllDigits() and the format comment on save().
        bool hasMaterialName =
            fields.size() > 3 &&
            !isAllDigits(fields[3]);

        int base =
            hasMaterialName ? 1 : 0;

        if (fields.size() < (size_t)(9 + base))
        {
            continue;
        }

        auto order = make_unique<ProcurementOrder>(
            fields[0],
            fields[1],
            fields[2],
            atoi(fields[3 + base].c_str()),
            fields[4 + base],
            atoi(fields[5 + base].c_str()),
            fields[6 + base]);

        order->setMaterialName(
            hasMaterialName ? fields[3] : "");

        order->setConfirmationDate(
            fields[7 + base]);

        order->setConfirmedQuantity(
            atoi(fields[8 + base].c_str()));

        if (fields.size() > (size_t)(9 + base) &&
            !fields[9 + base].empty())
        {
            vector<string> receiptTokens =
                splitBy(fields[9 + base], '~');

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

        if (fields.size() > (size_t)(10 + base))
        {
            order->setProjectionID(
                fields[10 + base]);
        }

        if (fields.size() > (size_t)(11 + base))
        {
            order->setClosed(
                fields[11 + base] == "1");
        }

        if (fields.size() > (size_t)(12 + base))
        {
            order->setCancelled(
                fields[12 + base] == "1");
        }

        // Backfill the name for a pre-materialName row from the
        // current Material catalog, so it reads correctly right away
        // and the row upgrades itself to the new layout on next save.
        if (!hasMaterialName &&
            materialManager != nullptr)
        {
            Material* material =
                materialManager->findMaterial(
                    order->getMaterialID());

            if (material != nullptr)
            {
                order->setMaterialName(
                    material->getName());
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
