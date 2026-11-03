#pragma once

#include <string>
#include <vector>
#include <mutex>

using namespace std;

// ================================================================
// MOVEMENT LEDGER
// ================================================================
// One single file records everything that happens in the Warehouse:
// stock movements (goods receipts / issues, transfers, product
// sales, production consumption, inventory adjustments) AND business
// events (procurement milestones, projections, inventory counts,
// purchase invoices). It is a plain CSV (UTF-8 with BOM) that Excel
// opens with a double click: data/movement_ledger.csv
//
// Columns: Timestamp, User, Type, Warehouse, Material ID, Material,
//          Product ID, Product, Qty In, Qty Out, Balance After,
//          Reference, Details, Comment
//
// "Balance After" is the stock of that material in that Warehouse
// right after the movement (empty for events and product rows).
//
// If the file cannot be written (e.g. it is open in Excel) the rows
// wait in memory and are written with the next movement.

struct LedgerRow
{
    string timestamp;
    string user;
    string type;
    string warehouse;
    string materialID;
    string materialName;
    string productID;
    string productName;
    string qtyIn;
    string qtyOut;
    string balance;
    string reference;
    string details;
    string comment;
};

class MovementLogger
{
private:

    string filename;
    string legacyFilename;
    bool logDataOperations;

    // Rows that could not be written yet (file locked)
    vector<string> pending;

    mutable recursive_mutex mutex;

    // Who is doing the current action (set per web request)
    static string& actorStorage();

    void appendRow(const LedgerRow& row);

    void writeLines(const vector<string>& lines);

    void importLegacyHistory();

public:

    // Constructor
    MovementLogger(
        string file = "data/movement_ledger.csv",
        string legacyFile = "data/movement_history.txt");

    // User recorded on every row written from this thread
    static void setActor(const string& name);

    static string getActor();

    // Business event (always recorded)
    void log(
        const string& type,
        const string& details,
        const string& comment = "");

    // Save/Load event - only recorded when "Log data operations"
    // is on
    void logSystemEvent(
        const string& type,
        const string& details);

    // Stock movement of a material (qtyIn or qtyOut, the other 0)
    // or a product row (materialID empty, productID set).
    // balance < 0 means "not applicable".
    void logStock(
        const string& type,
        int warehouseID,
        const string& materialID,
        const string& materialName,
        double qtyIn,
        double qtyOut,
        double balance,
        const string& reference,
        const string& comment = "",
        const string& productID = "",
        const string& productName = "",
        const string& details = "");

    // Enable / disable data save/load events
    void setLogDataOperations(
        bool enabled);

    bool getLogDataOperations() const;

    // Every row of the ledger, oldest first
    vector<LedgerRow> readAll() const;

    // Wipes the ledger (Data Management's full history reset) and
    // writes a single line marking when/that it happened.
    bool clearHistory();
};
