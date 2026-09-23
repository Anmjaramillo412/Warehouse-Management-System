#pragma once

#include <string>

using namespace std;

class MovementLogger
{
private:

    string filename;
    bool logDataOperations;

public:

    // Constructor
    MovementLogger(
        string file = "data/movement_history.txt");

    // Log inventory movement
    void log(
        const string& type,
        const string& details,
        const string& comment = "");

    // Log data save/load event
    void logSystemEvent(
        const string& type,
        const string& details);

    // Enable / disable ata save/load event
    void setLogDataOperations(
        bool enabled);

    bool getLogDataOperations() const;

    // Wipes the movement history file outright (used by Data
    // Management's full history reset, alongside Procurement/
    // Purchase/Projections clear()) and writes a single line marking
    // when/that it happened - unconditionally, regardless of the
    // logDataOperations setting, since the reset itself is always
    // worth a record.
    bool clearHistory();
};