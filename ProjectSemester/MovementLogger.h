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
};