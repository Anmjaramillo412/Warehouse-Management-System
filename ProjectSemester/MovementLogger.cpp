#include "MovementLogger.h"
#include "Quantity.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <filesystem>

using namespace std;

namespace fs = std::filesystem;


static const char* LEDGER_HEADER =
    "Timestamp,User,Type,Warehouse,Material ID,Material,"
    "Product ID,Product,Qty In,Qty Out,Balance After,"
    "Reference,Details,Comment";


// ================================================================
// HELPERS
// ================================================================

static string getCurrentTimestamp()
{
    auto now =
        chrono::system_clock::now();

    time_t currentTime =
        chrono::system_clock::to_time_t(
            now);

    tm localTime{};

#ifdef _WIN32

    localtime_s(
        &localTime,
        &currentTime);

#else

    localtime_r(
        &currentTime,
        &localTime);

#endif

    ostringstream timestamp;

    timestamp << put_time(
        &localTime,
        "%Y-%m-%d %H:%M:%S");

    return timestamp.str();
}


static string csvField(const string& value)
{
    string cleaned;

    for (char c : value)
    {
        if (c == '\r' || c == '\n')
        {
            cleaned += ' ';
        }
        else
        {
            cleaned += c;
        }
    }

    if (cleaned.find_first_of(",\"") == string::npos)
    {
        return cleaned;
    }

    string quoted = "\"";

    for (char c : cleaned)
    {
        if (c == '"')
        {
            quoted += "\"\"";
        }
        else
        {
            quoted += c;
        }
    }

    quoted += "\"";

    return quoted;
}


static string rowToLine(const LedgerRow& row)
{
    return
        csvField(row.timestamp) + "," +
        csvField(row.user) + "," +
        csvField(row.type) + "," +
        csvField(row.warehouse) + "," +
        csvField(row.materialID) + "," +
        csvField(row.materialName) + "," +
        csvField(row.productID) + "," +
        csvField(row.productName) + "," +
        csvField(row.qtyIn) + "," +
        csvField(row.qtyOut) + "," +
        csvField(row.balance) + "," +
        csvField(row.reference) + "," +
        csvField(row.details) + "," +
        csvField(row.comment);
}


// Splits one CSV record (the writer never puts a line break inside
// a field).
static vector<string> splitCsvLine(const string& line)
{
    vector<string> fields;

    string current;

    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); i++)
    {
        char c = line[i];

        if (inQuotes)
        {
            if (c == '"')
            {
                if (i + 1 < line.size() && line[i + 1] == '"')
                {
                    current += '"';

                    i++;
                }
                else
                {
                    inQuotes = false;
                }
            }
            else
            {
                current += c;
            }
        }
        else if (c == '"')
        {
            inQuotes = true;
        }
        else if (c == ',')
        {
            fields.push_back(current);

            current.clear();
        }
        else
        {
            current += c;
        }
    }

    fields.push_back(current);

    return fields;
}


string& MovementLogger::actorStorage()
{
    static thread_local string actor;

    return actor;
}


void MovementLogger::setActor(const string& name)
{
    actorStorage() = name;
}


string MovementLogger::getActor()
{
    return actorStorage();
}


// ================================================================
// CONSTRUCTOR
// ================================================================

MovementLogger::MovementLogger(
    string file,
    string legacyFile)
{
    filename = file;

    legacyFilename = legacyFile;

    logDataOperations = false;

    fs::path path(filename);

    if (!path.parent_path().empty())
    {
        fs::create_directories(
            path.parent_path());
    }

    // First start with the ledger: bring the old plain-text history
    // across so nothing that was recorded before is lost.
    if (!fs::exists(filename))
    {
        importLegacyHistory();
    }
}


// ================================================================
// WRITING
// ================================================================

void MovementLogger::writeLines(
    const vector<string>& lines)
{
    bool needHeader =
        !fs::exists(filename) ||
        fs::file_size(filename) == 0;

    ofstream file(
        filename,
        ios::app | ios::binary);

    if (!file.is_open())
    {
        // Locked (probably open in Excel) - keep for the next write
        for (const string& line : lines)
        {
            pending.push_back(line);
        }

        return;
    }

    if (needHeader)
    {
        file << "\xEF\xBB\xBF" << LEDGER_HEADER << "\r\n";
    }

    for (const string& line : lines)
    {
        file << line << "\r\n";
    }

    file.close();
}


void MovementLogger::appendRow(
    const LedgerRow& row)
{
    lock_guard<recursive_mutex> guard(mutex);

    LedgerRow stamped = row;

    if (stamped.timestamp.empty())
    {
        stamped.timestamp = getCurrentTimestamp();
    }

    if (stamped.user.empty())
    {
        string actor = getActor();

        stamped.user = actor.empty() ? "system" : actor;
    }

    vector<string> lines = pending;

    pending.clear();

    lines.push_back(rowToLine(stamped));

    writeLines(lines);
}


// ================================================================
// LOG EVENT
// ================================================================

void MovementLogger::log(
    const string& type,
    const string& details,
    const string& comment)
{
    LedgerRow row;

    row.type = type;

    row.details = details;

    row.comment = comment;

    appendRow(row);
}


void MovementLogger::logSystemEvent(
    const string& type,
    const string& details)
{
    if (!logDataOperations)
    {
        return;
    }

    LedgerRow row;

    row.type = type;

    row.details = details;

    appendRow(row);
}


// ================================================================
// LOG STOCK MOVEMENT / PRODUCT ROW
// ================================================================

void MovementLogger::logStock(
    const string& type,
    int warehouseID,
    const string& materialID,
    const string& materialName,
    double qtyIn,
    double qtyOut,
    double balance,
    const string& reference,
    const string& comment,
    const string& productID,
    const string& productName,
    const string& details)
{
    LedgerRow row;

    row.type = type;

    row.warehouse =
        warehouseID > 0 ? to_string(warehouseID) : "";

    row.materialID = materialID;

    row.materialName = materialName;

    row.productID = productID;

    row.productName = productName;

    row.qtyIn = qtyIn != 0.0 ? formatQty(qtyIn) : "";

    row.qtyOut = qtyOut != 0.0 ? formatQty(qtyOut) : "";

    row.balance = balance >= 0.0 ? formatQty(balance) : "";

    row.reference = reference;

    row.details = details;

    row.comment = comment;

    appendRow(row);
}


// ================================================================
// SETTINGS
// ================================================================

void MovementLogger::setLogDataOperations(
    bool enabled)
{
    logDataOperations =
        enabled;
}

bool MovementLogger::getLogDataOperations() const
{
    return logDataOperations;
}


// ================================================================
// READ
// ================================================================

vector<LedgerRow> MovementLogger::readAll() const
{
    lock_guard<recursive_mutex> guard(mutex);

    vector<LedgerRow> rows;

    ifstream file(filename, ios::binary);

    if (!file.is_open())
    {
        return rows;
    }

    string line;

    bool first = true;

    while (getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (first)
        {
            first = false;

            // header row (after the BOM)
            continue;
        }

        if (line.empty())
        {
            continue;
        }

        vector<string> f = splitCsvLine(line);

        f.resize(14);

        LedgerRow row;

        row.timestamp = f[0];
        row.user = f[1];
        row.type = f[2];
        row.warehouse = f[3];
        row.materialID = f[4];
        row.materialName = f[5];
        row.productID = f[6];
        row.productName = f[7];
        row.qtyIn = f[8];
        row.qtyOut = f[9];
        row.balance = f[10];
        row.reference = f[11];
        row.details = f[12];
        row.comment = f[13];

        rows.push_back(row);
    }

    return rows;
}


// ================================================================
// CLEAR HISTORY
// ================================================================

bool MovementLogger::clearHistory()
{
    lock_guard<recursive_mutex> guard(mutex);

    pending.clear();

    ofstream file(
        filename,
        ios::trunc | ios::binary);

    if (!file.is_open())
    {
        return false;
    }

    LedgerRow row;

    row.timestamp = getCurrentTimestamp();

    string actor = getActor();

    row.user = actor.empty() ? "system" : actor;

    row.type = "HISTORY RESET";

    row.details = "Movement history cleared for testing.";

    file << "\xEF\xBB\xBF" << LEDGER_HEADER << "\r\n"
        << rowToLine(row) << "\r\n";

    return true;
}


// ================================================================
// IMPORT OF THE OLD movement_history.txt
// ================================================================
// Old lines look like:
//   2026-09-28 11:49:32 | GOODS RECEIPT | WH: 1 | Material: X | Qty: 5 | Comment: ...
// They keep their original timestamp; goods receipts / issues and
// inventory adjustments become real stock rows so they show up in
// the material history. The old file is left untouched.

void MovementLogger::importLegacyHistory()
{
    ifstream in(legacyFilename);

    if (!in.is_open())
    {
        return;
    }

    vector<string> lines;

    string line;

    while (getline(in, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (line.empty())
        {
            continue;
        }

        // split by " | "
        vector<string> parts;

        size_t start = 0;

        while (true)
        {
            size_t pos = line.find(" | ", start);

            if (pos == string::npos)
            {
                parts.push_back(line.substr(start));

                break;
            }

            parts.push_back(line.substr(start, pos - start));

            start = pos + 3;
        }

        if (parts.size() < 2)
        {
            continue;
        }

        LedgerRow row;

        row.timestamp = parts[0];

        row.user = "(before ledger)";

        row.type = parts[1];

        string details;

        double qty = 0.0;

        bool hasQty = false;

        double counted = -1.0;

        double difference = 0.0;

        bool hasDifference = false;

        for (size_t i = 2; i < parts.size(); i++)
        {
            const string& part = parts[i];

            auto value = [&](const string& key) -> string
            {
                return part.substr(key.size());
            };

            try
            {
                if (part.rfind("WH: ", 0) == 0 &&
                    part.find("->") == string::npos)
                {
                    row.warehouse = value("WH: ");

                    continue;
                }

                if (part.rfind("Material: ", 0) == 0)
                {
                    row.materialID = value("Material: ");

                    continue;
                }

                if (part.rfind("Qty: ", 0) == 0 &&
                    (row.type == "GOODS RECEIPT" ||
                        row.type == "GOODS ISSUE"))
                {
                    qty = stod(value("Qty: "));

                    hasQty = true;

                    continue;
                }

                if (part.rfind("Counted: ", 0) == 0 &&
                    row.type == "INVENTORY ADJUSTMENT")
                {
                    counted = stod(value("Counted: "));

                    continue;
                }

                if (part.rfind("Difference: ", 0) == 0 &&
                    row.type == "INVENTORY ADJUSTMENT")
                {
                    difference = stod(value("Difference: "));

                    hasDifference = true;

                    continue;
                }

                if (part.rfind("Count: ", 0) == 0 &&
                    row.type == "INVENTORY ADJUSTMENT")
                {
                    row.reference = value("Count: ");

                    continue;
                }

                if (part.rfind("Comment: ", 0) == 0)
                {
                    row.comment = value("Comment: ");

                    continue;
                }
            }
            catch (...)
            {
            }

            if (!details.empty())
            {
                details += " | ";
            }

            details += part;
        }

        row.details = details;

        if (row.type == "GOODS RECEIPT" && hasQty)
        {
            row.qtyIn = formatQty(qty);
        }
        else if (row.type == "GOODS ISSUE" && hasQty)
        {
            row.qtyOut = formatQty(qty);
        }
        else if (row.type == "INVENTORY ADJUSTMENT" && hasDifference)
        {
            if (difference > 0)
            {
                row.qtyIn = formatQty(difference);
            }
            else
            {
                row.qtyOut = formatQty(-difference);
            }

            if (counted >= 0.0)
            {
                row.balance = formatQty(counted);
            }
        }

        lines.push_back(rowToLine(row));
    }

    if (!lines.empty())
    {
        writeLines(lines);
    }
}
