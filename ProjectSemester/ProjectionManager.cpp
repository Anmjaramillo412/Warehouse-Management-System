#include "ProjectionManager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <ctime>

using namespace std;

namespace fs = std::filesystem;


// ================================================================
// TODAY (ISO "YYYY-MM-DD"), for deciding whether a Projection's
// deadline has passed - see getOpenRequiredQuantity().
// ================================================================

static string todayISO()
{
    time_t now = time(nullptr);

    tm localTm = *localtime(&now);

    char buffer[11];

    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d",
        &localTm);

    return string(buffer);
}


// ================================================================
// FIELD SANITIZING (free text fields must not break the format)
// ================================================================

static string sanitizeProjectionField(const string& value)
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

static vector<string> splitProjectionField(
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

ProjectionManager::ProjectionManager(
    ProductManager* prodManager,
    WarehouseManager* whManager,
    InventoryManager* invManager,
    MovementLogger* logger,
    string file,
    MaterialManager* matManager)
{
    productManager = prodManager;
    warehouseManager = whManager;
    inventoryManager = invManager;
    movementLogger = logger;
    materialManager = matManager;

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
// SETTERS - DEPENDENCIES
// ================================================================

void ProjectionManager::setProductManager(
    ProductManager* manager)
{
    productManager = manager;
}


void ProjectionManager::setWarehouseManager(
    WarehouseManager* manager)
{
    warehouseManager = manager;
}


void ProjectionManager::setInventoryManager(
    InventoryManager* manager)
{
    inventoryManager = manager;
}


void ProjectionManager::setMovementLogger(
    MovementLogger* logger)
{
    movementLogger = logger;
}


void ProjectionManager::setMaterialManager(
    MaterialManager* manager)
{
    materialManager = manager;
}


// ================================================================
// GENERATE NEXT ID
// ================================================================

string ProjectionManager::generateNextID()
{
    ostringstream stream;

    stream << "PRJ-"
        << setfill('0')
        << setw(6)
        << nextNumber;

    nextNumber++;

    return stream.str();
}


// ================================================================
// GET WAREHOUSE STOCK (one material, one Warehouse)
// ================================================================

int ProjectionManager::getWarehouseStock(
    const string& materialID,
    int warehouseID) const
{
    if (warehouseManager == nullptr)
    {
        return 0;
    }

    Warehouse* warehouse =
        warehouseManager->findWarehouse(warehouseID);

    if (warehouse == nullptr)
    {
        return 0;
    }

    WarehouseNode* node =
        warehouse->findMaterial(materialID);

    return (node != nullptr) ? node->quantity : 0;
}


// ================================================================
// GET OPEN REQUIRED QUANTITY
// ================================================================
// Sum of this material's own requiredQuantity across every currently
// open Projection tied to this Warehouse. "Open" means not yet
// completed AND the deadline has not passed - a completed Projection
// already issued (consumed) its BOM from the Warehouse at completion
// time, so counting its requirement again here would subtract the
// same units a second time. An overdue-but-not-completed Projection
// is likewise dropped, on the assumption its plan was already acted on
// outside the system (goods issued/sold).

int ProjectionManager::getOpenRequiredQuantity(
    const string& materialID,
    int warehouseID) const
{
    int total = 0;

    string today =
        todayISO();

    for (const auto& projection : projections)
    {
        if (projection->getWarehouseID() != warehouseID)
        {
            continue;
        }

        if (projection->isCompleted())
        {
            continue;
        }

        const string& deadline =
            projection->getDeadline();

        bool stillActive =
            deadline.empty() ||
            deadline >= today;

        if (!stillActive)
        {
            continue;
        }

        for (const auto& item : projection->getItems())
        {
            if (item.materialID == materialID)
            {
                total += item.requiredQuantity;
            }
        }
    }

    return total;
}


// ================================================================
// GET QUANTITY TO ORDER
// ================================================================
// See the header comment on this function (ProjectionManager.h) for
// why this is a single shared number per material+Warehouse instead
// of a per-Projection calculation.

int ProjectionManager::getQuantityToOrder(
    const string& materialID,
    int warehouseID) const
{
    int required =
        getOpenRequiredQuantity(materialID, warehouseID);

    int stock =
        getWarehouseStock(materialID, warehouseID);

    int toOrder =
        required - stock;

    return (toOrder > 0) ? toOrder : 0;
}


// ================================================================
// CREATE PROJECTION
// ================================================================

Projection* ProjectionManager::createProjection(
    const string& productID,
    const string& deadline,
    int manufactureQuantity,
    const string& creationDate,
    string& errorMessage)
{
    errorMessage = "";

    if (manufactureQuantity <= 0)
    {
        errorMessage =
            "Quantity to Manufacture must be greater than zero.";

        return nullptr;
    }

    if (productManager == nullptr)
    {
        errorMessage = "Product not found.";

        return nullptr;
    }

    Product* product =
        productManager->findProduct(productID);

    if (product == nullptr)
    {
        errorMessage = "Product not found.";

        return nullptr;
    }

    if (product->getBOM().empty())
    {
        errorMessage =
            "This Product has no Bill of Materials.";

        return nullptr;
    }

    // The Warehouse is no longer picked by the caller - it is always
    // this Product's own Main Warehouse (set from Modify Product), so
    // every Projection for this Product always checks stock against
    // the same, single Warehouse.

    int warehouseID =
        product->getMainWarehouseID();

    if (warehouseID == 0)
    {
        errorMessage =
            "This Product has no Main Warehouse assigned yet. "
            "Please assign one from Modify Product before creating "
            "a Projection.";

        return nullptr;
    }

    string id =
        generateNextID();

    auto projection = make_unique<Projection>(
        id,
        productID,
        warehouseID,
        deadline,
        manufactureQuantity,
        creationDate);

    for (const auto& bomItem : product->getBOM())
    {
        // A BOM material supplied by an Internal Supplier (e.g.
        // "4Tex GmbH") is internal work, not a real purchase - it is
        // skipped entirely here, the same way it is refused at
        // Procurement Order creation (see WebServer.cpp).

        if (materialManager != nullptr)
        {
            Material* bomMaterial =
                materialManager->findMaterial(
                    bomItem.materialID);

            if (bomMaterial != nullptr &&
                bomMaterial->hasInternalSupplier())
            {
                continue;
            }
        }

        int required =
            bomItem.quantity * manufactureQuantity;

        // Every OTHER currently open Projection already competing for
        // this material in this same Warehouse - this new Projection
        // is not in the list yet, so it is naturally not counted
        // twice here; its own "required" is added on top below.

        int alreadyOpen =
            getOpenRequiredQuantity(
                bomItem.materialID,
                warehouseID);

        int stock =
            getWarehouseStock(
                bomItem.materialID,
                warehouseID);

        int shortfall =
            (alreadyOpen + required) - stock;

        if (shortfall > 0)
        {
            projection->addItem(
                bomItem.materialID,
                required,
                stock);
        }
    }

    if (projection->getItems().empty())
    {
        // Enough stock for everything - nothing worth recording.
        errorMessage =
            "Nothing to order: enough stock for all materials.";

        return nullptr;
    }

    Projection* result =
        projection.get();

    projections.push_back(
        std::move(projection));

    save();

    if (movementLogger != nullptr)
    {
        movementLogger->logSystemEvent(
            "PROJECTION CREATED",
            "ID: " + id +
            " | Product: " + productID +
            " | Qty: " + to_string(manufactureQuantity) +
            " | Deadline: " + deadline);
    }

    return result;
}


// ================================================================
// COMPLETE PROJECTION (confirm production, issue BOM materials)
// ================================================================
// The "perfect connection point": once every material is ordered,
// confirming how much was actually produced consumes the BOM
// materials from the Warehouse (Goods Issue) and archives the
// Projection - it stops reserving virtual stock and disappears from
// the active Projections list. Nothing is issued unless EVERY BOM
// material has enough actual stock for the full produced quantity.

bool ProjectionManager::completeProjection(
    const string& id,
    int producedQuantity,
    const string& completionDate)
{
    Projection* projection =
        findProjection(id);

    if (projection == nullptr)
    {
        return false;
    }

    if (projection->isCompleted())
    {
        return false;
    }

    if (producedQuantity <= 0)
    {
        return false;
    }

    if (productManager == nullptr ||
        warehouseManager == nullptr ||
        inventoryManager == nullptr)
    {
        return false;
    }

    Product* product =
        productManager->findProduct(
            projection->getProductID());

    if (product == nullptr ||
        product->getBOM().empty())
    {
        return false;
    }

    Warehouse* warehouse =
        warehouseManager->findWarehouse(
            projection->getWarehouseID());

    if (warehouse == nullptr)
    {
        return false;
    }

    // Check every material first - a partial Goods Issue halfway
    // through the BOM would leave inventory inconsistent.

    for (const auto& bomItem : product->getBOM())
    {
        int needed =
            bomItem.quantity * producedQuantity;

        WarehouseNode* node =
            warehouse->findMaterial(bomItem.materialID);

        int available =
            (node != nullptr) ? node->quantity : 0;

        if (available < needed)
        {
            return false;
        }
    }

    for (const auto& bomItem : product->getBOM())
    {
        int needed =
            bomItem.quantity * producedQuantity;

        inventoryManager->goodsIssue(
            projection->getWarehouseID(),
            bomItem.materialID,
            needed,
            "Production " + id);
    }

    projection->setCompleted(
        true,
        producedQuantity,
        completionDate);

    save();

    if (movementLogger != nullptr)
    {
        movementLogger->logSystemEvent(
            "PROJECTION COMPLETED",
            "ID: " + id +
            " | Produced Qty: " + to_string(producedQuantity));
    }

    return true;
}


// ================================================================
// REGISTER ITEM ORDER ATTEMPT
// ================================================================

bool ProjectionManager::registerItemOrderAttempt(
    const string& projectionID,
    const string& materialID,
    bool fullyCovered)
{
    Projection* projection =
        findProjection(projectionID);

    if (projection == nullptr)
    {
        return false;
    }

    bool found = false;

    for (const auto& item : projection->getItems())
    {
        if (item.materialID == materialID)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        return false;
    }

    projection->registerOrderAttempt(materialID, fullyCovered);

    save();

    return true;
}


// ================================================================
// FIND PROJECTION
// ================================================================

Projection* ProjectionManager::findProjection(
    const string& id)
{
    for (const auto& projection : projections)
    {
        if (projection->getID() == id)
        {
            return projection.get();
        }
    }

    return nullptr;
}


// ================================================================
// DELETE PROJECTION
// ================================================================

bool ProjectionManager::deleteProjection(
    const string& id)
{
    Projection* projection =
        findProjection(id);

    if (projection == nullptr)
    {
        return false;
    }

    if (projection->isCompleted())
    {
        return false;
    }

    projections.erase(
        remove_if(
            projections.begin(),
            projections.end(),
            [&id](const unique_ptr<Projection>& p)
            {
                return p->getID() == id;
            }),
        projections.end());

    save();

    if (movementLogger != nullptr)
    {
        movementLogger->logSystemEvent(
            "PROJECTION DELETED",
            "ID: " + id);
    }

    return true;
}


// ================================================================
// GET PROJECTIONS
// ================================================================

const vector<unique_ptr<Projection>>&
ProjectionManager::getProjections() const
{
    return projections;
}


// ================================================================
// SAVE
// ================================================================
// Plain text format, one line per projection:
//
// id|productID|warehouseID|deadline|manufactureQuantity|creationDate|items
//
// items: each as "materialID,requiredQuantity,stockAtCreation,
// orderRegistered,orderRegistrationCount", several joined by "~"

bool ProjectionManager::save()
{
    ofstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    for (const auto& projection : projections)
    {
        string itemsField = "";

        const auto& items =
            projection->getItems();

        for (size_t i = 0; i < items.size(); i++)
        {
            if (i > 0)
            {
                itemsField += "~";
            }

            itemsField +=
                items[i].materialID + "," +
                to_string(items[i].requiredQuantity) + "," +
                to_string(items[i].stockAtCreation) + "," +
                (items[i].orderRegistered ? "1" : "0") + "," +
                to_string(items[i].orderRegistrationCount);
        }

        file << projection->getID() << "|"
            << sanitizeProjectionField(projection->getProductID()) << "|"
            << projection->getWarehouseID() << "|"
            << projection->getDeadline() << "|"
            << projection->getManufactureQuantity() << "|"
            << projection->getCreationDate() << "|"
            << itemsField << "|"
            << (projection->isCompleted() ? "1" : "0") << "|"
            << projection->getProducedQuantity() << "|"
            << projection->getCompletionDate()
            << endl;
    }

    file.close();

    return true;
}


// ================================================================
// LOAD
// ================================================================

bool ProjectionManager::load()
{
    ifstream file(filename);

    if (!file.is_open())
    {
        // No file yet - not an error, just nothing to load.
        return true;
    }

    projections.clear();

    int highestNumber = 0;

    string line;

    while (getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        vector<string> fields =
            splitProjectionField(line, '|');

        if (fields.size() < 6)
        {
            continue;
        }

        auto projection = make_unique<Projection>(
            fields[0],
            fields[1],
            atoi(fields[2].c_str()),
            fields[3],
            atoi(fields[4].c_str()),
            fields[5]);

        if (fields.size() > 6 &&
            !fields[6].empty())
        {
            vector<string> itemTokens =
                splitProjectionField(fields[6], '~');

            for (const string& token : itemTokens)
            {
                vector<string> itemFields =
                    splitProjectionField(token, ',');

                if (itemFields.size() < 3)
                {
                    continue;
                }

                projection->addItem(
                    itemFields[0],
                    atoi(itemFields[1].c_str()),
                    atoi(itemFields[2].c_str()));

                if (itemFields.size() > 3)
                {
                    bool registered =
                        itemFields[3] == "1";

                    int registrationCount =
                        (itemFields.size() > 4) ?
                        atoi(itemFields[4].c_str()) : 0;

                    projection->setItemOrderState(
                        itemFields[0],
                        registered,
                        registrationCount);
                }
            }
        }

        if (fields.size() > 9)
        {
            bool completed =
                fields[7] == "1";

            int producedQuantity =
                atoi(fields[8].c_str());

            string completionDate =
                fields[9];

            projection->setCompleted(
                completed,
                producedQuantity,
                completionDate);
        }

        // Track the highest existing "PRJ-######" number so new
        // projections keep counting up instead of reusing IDs.

        string idNumberPart =
            fields[0].size() > 4 ?
            fields[0].substr(4) : "";

        int idNumber =
            atoi(idNumberPart.c_str());

        if (idNumber > highestNumber)
        {
            highestNumber = idNumber;
        }

        projections.push_back(
            std::move(projection));
    }

    file.close();

    nextNumber = highestNumber + 1;

    return true;
}


// ================================================================
// CLEAR
// ================================================================

void ProjectionManager::clear()
{
    projections.clear();

    nextNumber = 1;
}
