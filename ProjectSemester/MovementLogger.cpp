#include "MovementLogger.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <filesystem>

using namespace std;

namespace fs = std::filesystem;


// ================================================================
// CONSTRUCTOR
// ================================================================

MovementLogger::MovementLogger(
    string file)
{
    filename = file;

    logDataOperations = false;

    // Create parent directory if necessary

    fs::path path(filename);

    if (!path.parent_path().empty())
    {
        fs::create_directories(
            path.parent_path());
    }
}


// ================================================================
// GET CURRENT TIMESTAMP
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


// ================================================================
// LOG MOVEMENT
// ================================================================

void MovementLogger::log(
    const string& type,
    const string& details,
    const string& comment)
{
    ofstream file(
        filename,
        ios::app);


    if (!file.is_open())
    {
        return;
    }


    file << getCurrentTimestamp()
        << " | "
        << type
        << " | "
        << details;


    if (!comment.empty())
    {
        file << " | Comment: "
            << comment;
    }


    file << endl;


    file.close();
}


// ================================================================
// LOG SYSTEM EVENT
// ================================================================

void MovementLogger::logSystemEvent(
    const string& type,
    const string& details)
{
    if (!logDataOperations)
    {
        return;
    }


    ofstream file(
        filename,
        ios::app);


    if (!file.is_open())
    {
        return;
    }


    file << getCurrentTimestamp()
        << " | "
        << type
        << " | "
        << details
        << endl;


    file.close();
}

// ================================================================
// SET LOG DATA OPERATIONS
// ================================================================

void MovementLogger::setLogDataOperations(
    bool enabled)
{
    logDataOperations =
        enabled;
}

// ================================================================
// GET LOG DATA OPERATIONS
// ================================================================

bool MovementLogger::getLogDataOperations() const
{
    return logDataOperations;
}