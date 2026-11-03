#pragma once

// ================================================================
// INVENTORY COUNT MANAGER (header-only)
// ----------------------------------------------------------------
// A physical inventory count of ONE Warehouse, which may take more
// than one day:
//
//   Open      - draft. Counted quantities (and reasons / comments) can
//               be saved and completed on a later day. Stock is NOT
//               changed, and the Warehouse is locked: no goods
//               receipt / issue / transfer / sale / production in or
//               out of it until the count is applied or cancelled
//               (see InventoryManager::isWarehouseLocked()).
//   Applied   - final. Stock was set to the counted quantities; every
//               adjusted line is written to the Movement History as
//               "INVENTORY ADJUSTMENT". Can no longer be edited.
//   Cancelled - discarded without any effect on stock.
//
// Header-only on purpose: a new .cpp would also have to be added to
// the Visual Studio project.
// ================================================================

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

#include "Quantity.h"
#include "WarehouseManager.h"
#include "MaterialManager.h"
#include "MovementLogger.h"
#include "InventoryManager.h"

using namespace std;


// ================================================================
// COUNT LINE
// ================================================================

struct InventoryCountLine
{
    string materialID;

    // false = not counted yet (pending)
    bool counted = false;

    double countedQuantity = 0.0;

    string reason;

    string comment;

    // Filled in when the count is applied (the system quantity the
    // counted quantity was compared against, and the resulting
    // adjustment: counted - system).
    double systemQuantity = 0.0;

    double difference = 0.0;
};


// ================================================================
// COUNT
// ================================================================

struct InventoryCount
{
    string id;

    int warehouseID = 0;

    // "Open", "Applied" or "Cancelled"
    string status = "Open";

    // ISO dates (YYYY-MM-DD): when the count was started / finished
    string startDate;

    string endDate;

    string comment;

    vector<InventoryCountLine> lines;

    InventoryCountLine* findLine(const string& materialID)
    {
        for (auto& line : lines)
        {
            if (line.materialID == materialID)
            {
                return &line;
            }
        }

        return nullptr;
    }

    const InventoryCountLine* findLine(const string& materialID) const
    {
        for (const auto& line : lines)
        {
            if (line.materialID == materialID)
            {
                return &line;
            }
        }

        return nullptr;
    }
};


// ================================================================
// RESULT OF APPLYING A COUNT
// ================================================================

struct InventoryCountSummary
{
    int linesCounted = 0;

    int linesAdjusted = 0;

    int linesPending = 0;
};


// ================================================================
// MANAGER
// ================================================================

class InventoryCountManager
{
private:

    vector<unique_ptr<InventoryCount>> counts;

    int nextNumber = 1;

    string filename;

    WarehouseManager* warehouseManager;

    MaterialManager* materialManager;

    InventoryManager* inventoryManager;

    MovementLogger* movementLogger;


    // ------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------

    static string encodeField(const string& value)
    {
        string result;

        const char* hex = "0123456789ABCDEF";

        for (unsigned char c : value)
        {
            if (c == '%' || c == '|' || c == '~' || c == ',' ||
                c == '\n' || c == '\r')
            {
                result += '%';
                result += hex[c >> 4];
                result += hex[c & 15];
            }
            else
            {
                result += static_cast<char>(c);
            }
        }

        return result;
    }

    static string decodeField(const string& value)
    {
        string result;

        for (size_t i = 0; i < value.size(); i++)
        {
            if (value[i] == '%' && i + 2 < value.size() &&
                isxdigit(static_cast<unsigned char>(value[i + 1])) &&
                isxdigit(static_cast<unsigned char>(value[i + 2])))
            {
                result += static_cast<char>(
                    strtol(value.substr(i + 1, 2).c_str(),
                        nullptr, 16));

                i += 2;
            }
            else
            {
                result += value[i];
            }
        }

        return result;
    }

    // Keeps empty fields (std::getline loses a trailing empty one)
    static vector<string> splitKeepEmpty(
        const string& value,
        char delimiter)
    {
        vector<string> result;

        string current;

        for (char c : value)
        {
            if (c == delimiter)
            {
                result.push_back(current);

                current.clear();
            }
            else
            {
                current += c;
            }
        }

        result.push_back(current);

        return result;
    }

    string nextID()
    {
        ostringstream stream;

        stream << "CNT-";

        string number = to_string(nextNumber);

        stream << string(6 - min<size_t>(6, number.size()), '0')
            << number;

        nextNumber++;

        return stream.str();
    }

    // Material that can be counted: self-made (Internal Supplier)
    // materials are normally not stocked, but if one does have stock
    // in the Warehouse (e.g. loaded from Excel) it must be countable,
    // otherwise that stock could never be corrected.
    bool isCountable(const Material* material, int warehouseID) const
    {
        if (material == nullptr)
        {
            return false;
        }

        if (!material->hasInternalSupplier())
        {
            return true;
        }

        return currentStock(warehouseID, material->getID()) > 0.0;
    }

    double currentStock(
        int warehouseID,
        const string& materialID) const
    {
        if (warehouseManager == nullptr)
        {
            return 0.0;
        }

        Warehouse* warehouse =
            warehouseManager->findWarehouse(warehouseID);

        if (warehouse == nullptr)
        {
            return 0.0;
        }

        WarehouseNode* node =
            warehouse->findMaterial(materialID);

        return node != nullptr ? node->quantity : 0.0;
    }

    void log(
        const string& type,
        const string& details,
        const string& comment = "")
    {
        if (movementLogger != nullptr)
        {
            movementLogger->log(type, details, comment);
        }
    }

    // Warehouses with an Open count are locked in InventoryManager
    void syncLocks()
    {
        if (inventoryManager == nullptr)
        {
            return;
        }

        inventoryManager->clearWarehouseLocks();

        for (const auto& count : counts)
        {
            if (count->status == "Open")
            {
                inventoryManager->setWarehouseLocked(
                    count->warehouseID, true);
            }
        }
    }

    // "Missing material" needs a lower count, "Surplus material" a
    // higher one. Returns "" when the reason fits the difference.
    static string reasonMismatch(
        const string& reason,
        double difference,
        const string& materialID,
        double systemQuantity)
    {
        if (reason == "Missing material" && difference > 0)
        {
            return "Material " + materialID + ": reason \"Missing "
                "material\" needs a counted quantity below the system "
                "quantity (" + formatQty(systemQuantity) + ").";
        }

        if (reason == "Surplus material" && difference < 0)
        {
            return "Material " + materialID + ": reason \"Surplus "
                "material\" needs a counted quantity above the system "
                "quantity (" + formatQty(systemQuantity) + ").";
        }

        return "";
    }

public:

    // ------------------------------------------------------------
    // Reasons
    // ------------------------------------------------------------

    static vector<string> reasons()
    {
        return {
            "Missing material",
            "Surplus material",
            "Damaged / Scrap",
            "Registration error / Other",
            "Inventory adjustment"
        };
    }

    static bool isValidReason(const string& reason)
    {
        for (const auto& r : reasons())
        {
            if (r == reason)
            {
                return true;
            }
        }

        return false;
    }

    // Suggested reason when the user did not pick one
    static string defaultReason(double difference)
    {
        if (difference < 0)
        {
            return "Missing material";
        }

        if (difference > 0)
        {
            return "Surplus material";
        }

        return "";
    }

    static bool isValidDate(const string& date)
    {
        if (date.size() != 10 || date[4] != '-' || date[7] != '-')
        {
            return false;
        }

        for (size_t i = 0; i < date.size(); i++)
        {
            if (i == 4 || i == 7)
            {
                continue;
            }

            if (!isdigit(static_cast<unsigned char>(date[i])))
            {
                return false;
            }
        }

        int month = atoi(date.substr(5, 2).c_str());

        int day = atoi(date.substr(8, 2).c_str());

        return month >= 1 && month <= 12 && day >= 1 && day <= 31;
    }


    // ------------------------------------------------------------
    // Constructor
    // ------------------------------------------------------------

    InventoryCountManager(
        WarehouseManager* whManager = nullptr,
        MaterialManager* matManager = nullptr,
        InventoryManager* invManager = nullptr,
        MovementLogger* logger = nullptr,
        string file = "data/inventory_counts.txt")
    {
        warehouseManager = whManager;

        materialManager = matManager;

        inventoryManager = invManager;

        movementLogger = logger;

        filename = file;

        load();
    }


    // ------------------------------------------------------------
    // Access
    // ------------------------------------------------------------

    const vector<unique_ptr<InventoryCount>>& getCounts() const
    {
        return counts;
    }

    InventoryCount* findCount(const string& id)
    {
        for (auto& count : counts)
        {
            if (count->id == id)
            {
                return count.get();
            }
        }

        return nullptr;
    }

    // ID of the Open count of this Warehouse ("" when none)
    string getOpenCountID(int warehouseID) const
    {
        for (const auto& count : counts)
        {
            if (count->status == "Open" &&
                count->warehouseID == warehouseID)
            {
                return count->id;
            }
        }

        return "";
    }

    // Live stock (used by the screens while a count is Open)
    double getSystemQuantity(
        int warehouseID,
        const string& materialID) const
    {
        return currentStock(warehouseID, materialID);
    }


    // ------------------------------------------------------------
    // Save / load (data/inventory_counts.txt)
    // ------------------------------------------------------------
    // One line per count:
    //   id|warehouseID|status|startDate|endDate|comment|lines
    // lines joined by "~", each:
    //   materialID,counted,countedQty,reason,comment,systemQty,difference
    // (free text is %-encoded so it can contain any character)

    bool save()
    {
        ofstream file(filename);

        if (!file.is_open())
        {
            return false;
        }

        for (const auto& count : counts)
        {
            string linesField;

            for (size_t i = 0; i < count->lines.size(); i++)
            {
                const InventoryCountLine& line = count->lines[i];

                if (i > 0)
                {
                    linesField += "~";
                }

                linesField +=
                    encodeField(line.materialID) + "," +
                    (line.counted ? "1" : "0") + "," +
                    formatQty(line.countedQuantity) + "," +
                    encodeField(line.reason) + "," +
                    encodeField(line.comment) + "," +
                    formatQty(line.systemQuantity) + "," +
                    formatQty(line.difference);
            }

            file << count->id << "|"
                << count->warehouseID << "|"
                << count->status << "|"
                << count->startDate << "|"
                << count->endDate << "|"
                << encodeField(count->comment) << "|"
                << linesField
                << endl;
        }

        return true;
    }

    bool load()
    {
        ifstream file(filename);

        if (!file.is_open())
        {
            // No file yet - nothing to load
            syncLocks();

            return true;
        }

        counts.clear();

        int highest = 0;

        string text;

        while (getline(file, text))
        {
            if (text.empty())
            {
                continue;
            }

            if (text.back() == '\r')
            {
                text.pop_back();
            }

            vector<string> fields = splitKeepEmpty(text, '|');

            if (fields.size() < 7)
            {
                continue;
            }

            auto count = make_unique<InventoryCount>();

            count->id = fields[0];

            count->warehouseID = atoi(fields[1].c_str());

            count->status = fields[2];

            count->startDate = fields[3];

            count->endDate = fields[4];

            count->comment = decodeField(fields[5]);

            if (!fields[6].empty())
            {
                for (const string& lineText :
                    splitKeepEmpty(fields[6], '~'))
                {
                    vector<string> parts =
                        splitKeepEmpty(lineText, ',');

                    if (parts.size() < 7)
                    {
                        continue;
                    }

                    InventoryCountLine line;

                    line.materialID = decodeField(parts[0]);

                    line.counted = (parts[1] == "1");

                    line.countedQuantity = parseQty(parts[2]);

                    line.reason = decodeField(parts[3]);

                    line.comment = decodeField(parts[4]);

                    line.systemQuantity = parseQty(parts[5]);

                    line.difference = parseQty(parts[6]);

                    count->lines.push_back(line);
                }
            }

            if (count->id.size() > 4 &&
                count->id.compare(0, 4, "CNT-") == 0)
            {
                highest = max(highest,
                    atoi(count->id.substr(4).c_str()));
            }

            counts.push_back(std::move(count));
        }

        nextNumber = highest + 1;

        syncLocks();

        return true;
    }


    // ------------------------------------------------------------
    // Open a new count
    // ------------------------------------------------------------
    // Lists every material currently stocked in the Warehouse as a
    // pending line, locks the Warehouse.

    InventoryCount* createCount(
        int warehouseID,
        const string& startDate,
        const string& comment,
        string& error)
    {
        error = "";

        if (warehouseManager == nullptr ||
            warehouseManager->findWarehouse(warehouseID) == nullptr)
        {
            error = "Warehouse not found.";

            return nullptr;
        }

        if (!isValidDate(startDate))
        {
            error = "Please enter a valid Start Date.";

            return nullptr;
        }

        string openID = getOpenCountID(warehouseID);

        if (!openID.empty())
        {
            error = "Warehouse " + to_string(warehouseID) +
                " already has an open inventory count (" + openID +
                "). Finish or cancel it first.";

            return nullptr;
        }

        auto count = make_unique<InventoryCount>();

        count->id = nextID();

        count->warehouseID = warehouseID;

        count->status = "Open";

        count->startDate = startDate;

        count->comment = comment;

        Warehouse* warehouse =
            warehouseManager->findWarehouse(warehouseID);

        for (WarehouseNode* node = warehouse->getHead();
            node != nullptr;
            node = node->next)
        {
            if (node->material == nullptr ||
                !isCountable(node->material, warehouseID))
            {
                continue;
            }

            InventoryCountLine line;

            line.materialID = node->material->getID();

            count->lines.push_back(line);
        }

        sort(count->lines.begin(), count->lines.end(),
            [](const InventoryCountLine& a, const InventoryCountLine& b)
            {
                return a.materialID < b.materialID;
            });

        InventoryCount* result = count.get();

        counts.push_back(std::move(count));

        save();

        syncLocks();

        log("INVENTORY COUNT OPENED",
            "Count: " + result->id +
            " | WH: " + to_string(warehouseID) +
            " | Start: " + startDate +
            " | Materials listed: " +
            to_string(result->lines.size()),
            comment);

        return result;
    }


    // ------------------------------------------------------------
    // Save the draft (counted quantities, reasons, comments)
    // ------------------------------------------------------------
    // Lines for a material that is not in the count yet (e.g. one
    // with stock 0 in this Warehouse) are added. All lines are
    // validated first; nothing is changed if one of them is invalid.

    bool saveDraft(
        const string& id,
        const string& comment,
        const vector<InventoryCountLine>& input,
        string& error)
    {
        error = "";

        InventoryCount* count = findCount(id);

        if (count == nullptr)
        {
            error = "Inventory count not found.";

            return false;
        }

        if (count->status != "Open")
        {
            error = "Only an open inventory count can be changed.";

            return false;
        }

        vector<InventoryCountLine> prepared;

        for (const InventoryCountLine& source : input)
        {
            InventoryCountLine line = source;

            Material* material =
                materialManager != nullptr ?
                materialManager->findMaterial(line.materialID) :
                nullptr;

            if (material == nullptr)
            {
                error = "Material not found: " + line.materialID;

                return false;
            }

            if (!isCountable(material, count->warehouseID))
            {
                error = "Material " + line.materialID +
                    " is made by an Internal Supplier and has no "
                    "stock here, so it cannot be counted.";

                return false;
            }

            if (!line.reason.empty() && !isValidReason(line.reason))
            {
                error = "Invalid reason for material " +
                    line.materialID + ".";

                return false;
            }

            if (line.counted)
            {
                if (!hasValidQtyDecimals(line.countedQuantity) ||
                    line.countedQuantity < 0)
                {
                    error = "Counted quantity of " + line.materialID +
                        " must be zero or more, with at most "
                        "3 decimals.";

                    return false;
                }

                line.countedQuantity = roundQty(line.countedQuantity);

                double system =
                    currentStock(count->warehouseID, line.materialID);

                double difference =
                    roundQty(line.countedQuantity - system);

                if (difference == 0.0)
                {
                    line.reason = "";
                }
                else if (line.reason.empty())
                {
                    line.reason = defaultReason(difference);
                }

                string mismatch =
                    reasonMismatch(
                        line.reason,
                        difference,
                        line.materialID,
                        system);

                if (!mismatch.empty())
                {
                    error = mismatch;

                    return false;
                }
            }
            else
            {
                line.countedQuantity = 0.0;

                line.reason = "";
            }

            prepared.push_back(line);
        }

        // All valid - apply
        for (const InventoryCountLine& line : prepared)
        {
            InventoryCountLine* existing =
                count->findLine(line.materialID);

            if (existing == nullptr)
            {
                count->lines.push_back(InventoryCountLine());

                existing = &count->lines.back();

                existing->materialID = line.materialID;
            }

            existing->counted = line.counted;

            existing->countedQuantity = line.countedQuantity;

            existing->reason = line.reason;

            existing->comment = line.comment;
        }

        sort(count->lines.begin(), count->lines.end(),
            [](const InventoryCountLine& a, const InventoryCountLine& b)
            {
                return a.materialID < b.materialID;
            });

        count->comment = comment;

        save();

        return true;
    }


    // ------------------------------------------------------------
    // Apply the count: stock becomes the counted quantities
    // ------------------------------------------------------------

    bool applyCount(
        const string& id,
        const string& endDate,
        InventoryCountSummary& summary,
        string& error)
    {
        error = "";

        summary = InventoryCountSummary();

        InventoryCount* count = findCount(id);

        if (count == nullptr)
        {
            error = "Inventory count not found.";

            return false;
        }

        if (count->status != "Open")
        {
            error = "Only an open inventory count can be applied.";

            return false;
        }

        if (!isValidDate(endDate))
        {
            error = "Please enter a valid End Date.";

            return false;
        }

        if (endDate < count->startDate)
        {
            error = "The End Date cannot be before the Start Date (" +
                count->startDate + ").";

            return false;
        }

        Warehouse* warehouse =
            warehouseManager != nullptr ?
            warehouseManager->findWarehouse(count->warehouseID) :
            nullptr;

        if (warehouse == nullptr)
        {
            error = "Warehouse not found.";

            return false;
        }

        // Validate everything before touching stock
        for (const InventoryCountLine& line : count->lines)
        {
            if (!line.counted)
            {
                summary.linesPending++;

                continue;
            }

            summary.linesCounted++;

            double system =
                currentStock(count->warehouseID, line.materialID);

            double difference =
                roundQty(line.countedQuantity - system);

            string reason = line.reason;

            if (difference == 0.0)
            {
                continue;
            }

            if (reason.empty())
            {
                reason = defaultReason(difference);
            }

            string mismatch =
                reasonMismatch(
                    reason,
                    difference,
                    line.materialID,
                    system);

            if (!mismatch.empty())
            {
                error = mismatch;

                return false;
            }
        }

        if (summary.linesCounted == 0)
        {
            error = "Nothing has been counted yet.";

            return false;
        }

        summary.linesAdjusted = 0;

        // Apply
        for (InventoryCountLine& line : count->lines)
        {
            double system =
                currentStock(count->warehouseID, line.materialID);

            line.systemQuantity = system;

            if (!line.counted)
            {
                line.difference = 0.0;

                continue;
            }

            double difference =
                roundQty(line.countedQuantity - system);

            line.difference = difference;

            if (difference == 0.0)
            {
                line.reason = "";

                continue;
            }

            if (line.reason.empty())
            {
                line.reason = defaultReason(difference);
            }

            Material* material =
                materialManager->findMaterial(line.materialID);

            bool changed = false;

            if (difference > 0)
            {
                changed =
                    warehouse->addMaterial(material, difference);
            }
            else
            {
                changed =
                    warehouse->removeMaterial(
                        line.materialID, -difference);
            }

            if (!changed)
            {
                // Cannot happen after the checks above; keep the
                // data consistent anyway.
                line.difference = 0.0;

                continue;
            }

            summary.linesAdjusted++;

            if (movementLogger != nullptr)
            {
                movementLogger->logStock(
                    "INVENTORY ADJUSTMENT",
                    count->warehouseID,
                    line.materialID,
                    material != nullptr ? material->getName() : "",
                    difference > 0 ? difference : 0.0,
                    difference < 0 ? -difference : 0.0,
                    line.countedQuantity,
                    count->id,
                    line.comment,
                    "",
                    "",
                    "System: " + formatQty(system) +
                    " | Counted: " + formatQty(line.countedQuantity) +
                    " | Reason: " + line.reason);
            }
        }

        count->status = "Applied";

        count->endDate = endDate;

        save();

        syncLocks();

        log("INVENTORY COUNT APPLIED",
            "Count: " + count->id +
            " | WH: " + to_string(count->warehouseID) +
            " | Start: " + count->startDate +
            " | End: " + endDate +
            " | Counted: " + to_string(summary.linesCounted) +
            " | Adjusted: " + to_string(summary.linesAdjusted) +
            " | Not counted: " + to_string(summary.linesPending),
            count->comment);

        return true;
    }


    // ------------------------------------------------------------
    // Cancel an open count (no effect on stock)
    // ------------------------------------------------------------

    bool cancelCount(
        const string& id,
        string& error)
    {
        error = "";

        InventoryCount* count = findCount(id);

        if (count == nullptr)
        {
            error = "Inventory count not found.";

            return false;
        }

        if (count->status != "Open")
        {
            error = "Only an open inventory count can be cancelled.";

            return false;
        }

        count->status = "Cancelled";

        save();

        syncLocks();

        log("INVENTORY COUNT CANCELLED",
            "Count: " + count->id +
            " | WH: " + to_string(count->warehouseID) +
            " | Start: " + count->startDate,
            count->comment);

        return true;
    }
};
