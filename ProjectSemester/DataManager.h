#pragma once

#include "MaterialManager.h"
#include "SupplierManager.h"
#include "WarehouseManager.h"
#include "ProductManager.h"
#include "MovementLogger.h"

#include <string>

using namespace std;

class DataManager
{
private:

    string filename;

    string configFilename;

    MovementLogger* movementLogger;

    // Whether Material/Supplier/Warehouse/Product data (everything
    // save()/load() below handle - the data the "Save Data" / "Load
    // Data" buttons work on) is saved automatically after every
    // operation that changes it (Goods Receipt/Issue/Transfer,
    // Procurement receiving, creating/deleting a Material, Supplier,
    // Warehouse or Product, selling a Product), and loaded
    // automatically when the server starts - see WebServer::run() and
    // autoSaveIfEnabled(). Off by default; turned on/off from Data
    // Management Settings, persisted across restarts.
    bool autoSaveAndLoad;

    void loadConfig();

    void saveConfig();

public:

    // Constructor
    DataManager(
        string file = "data/warehouse_data.xlsx",
        string configFile = "data/datamanager_config.txt");

    void setMovementLogger(
        MovementLogger* logger);

    // Save
    bool save(
        MaterialManager& materialManager,
        SupplierManager& supplierManager,
        WarehouseManager& warehouseManager,
        ProductManager& productManager);

    // Load
    bool load(
        MaterialManager& materialManager,
        SupplierManager& supplierManager,
        WarehouseManager& warehouseManager,
        ProductManager& productManager);

    // ------------------------------------------------------------
    // Auto Save and Load configuration
    // ------------------------------------------------------------

    bool getAutoSaveAndLoad() const;

    bool setAutoSaveAndLoad(bool enabled);
};
