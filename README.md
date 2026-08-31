# Warehouse Management System

## Project Description

This project is a C++-based Warehouse Management System.

The system provides functionality for managing materials, warehouses, inventory movements, products, Bills of Materials (BOMs), and persistent data storage.

The application combines an object-oriented C++ backend with a web-based user interface.

## Technologies

- C++
- Crow
- XLNT
- HTML
- CSS
- JavaScript
- Microsoft Excel

## Main Features

### Material Management

- Create materials
- Search materials
- Modify materials
- Delete materials
- Display material master data
- Material ID validation using the format `###-######`
- Material image management

### Warehouse Management

- Create warehouses
- Display warehouses
- Delete empty warehouses
- Prevent deletion of warehouses containing inventory

### Inventory Management

- Goods Receipt
- Goods Issue
- Material Transfer
- Inventory Display
- Inventory Integrity Check

### Product Management

- Create products
- Define Bills of Materials
- Display products
- Delete products
- Sell products using automatic BOM-based material consumption

### Data Persistence

System data is stored using XLNT in an Excel workbook containing:

- Materials
- Warehouses
- Inventory
- Products
- BOM

## Architecture

The application is structured into several classes with clearly separated responsibilities.

```text
WarehouseSystem
│
├── MaterialManager
│   └── Material
│
├── WarehouseManager
│   └── Warehouse
│       └── WarehouseNode
│
├── InventoryManager
│
├── ProductManager
│   └── Product
│       └── BOMItem
│
├── DataManager
│
└── WebServer
