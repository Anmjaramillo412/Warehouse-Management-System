#pragma once

#include "WarehouseSystem.h"

class ConsoleUI
{
private:

    WarehouseSystem system;

public:

    // Constructor
    ConsoleUI();

    // Start application
    void run();

private:

    // Main menu
    void mainMenu();

    // Material menu
    void materialMenu();

    Material readMaterial();

    void createMaterial();

    void modifyMaterial();

    void deleteMaterial();

    void searchMaterial();

    void displayMaterials();

    // Warehouse menu
    void warehouseMenu();

    void createWarehouse();

    void deleteWarehouse();

    void searchWarehouse();

    void displayWarehouses();

    void addMaterialToWarehouse();

    void removeMaterialFromWarehouse();

    void displayWarehouseInventory();

    // Inventory operations

    void inventoryMenu();

    void goodsReceipt();

    void goodsIssue();

    void transferMaterial();

    // Data 

    void saveData();

    void loadData();
};