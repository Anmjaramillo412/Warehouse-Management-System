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
- Delete materials (only allowed while not in use anywhere in the system)
- Display active material master data, with a separate Archived Materials view for materials marked inactive
- Material ID validation using the format `###-######` or `######-00`
- Material image management
- One primary Supplier per material, plus any number of additional Suppliers, each with its own Supplier Part Number

### Supplier Management

- Create, search, modify and delete suppliers
- Supplier details: address, country, contact, website, ordering methods, payment method, lead time
- Linked to Materials (as the primary Supplier or as an additional one) and used to group Procurement Orders by supplier

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

### Projection Management

- Create Projections (a planned production batch for a Product/Warehouse, with its own BOM requirement and deadline)
- Open Projections / Archived Projections views
- Virtual Stock per material: real warehouse stock minus what every other still-open Projection has already reserved
- Register Procurement Orders directly from a Projection's missing materials, with a one-click "Register Today" shortcut
- Delete a Projection
- Confirm Production once a Projection is Fully Ordered, issuing the BOM from inventory and archiving the Projection

### Procurement Management

- New Procurement Orders, either standalone or registered from a Projection, sharing one consecutive "PRC-######" number per action regardless of where they were created
- Open Orders (not yet confirmed by the supplier) / Confirmed Orders (confirmed, awaiting goods receipt) / Archived Orders (Completed, Closed (Incomplete), or Cancelled)
- Confirm an order (manually or with a one-click "Confirm as Ordered Today")
- Receive materials against a confirmed order, partially or in full, with a one-click "Receipt Today"
- Close an order that will never fully arrive - whatever was received stays in inventory, the rest stops counting as pending
- Cancel an order that was never confirmed and never will be
- Confirmed orders cannot be deleted, to keep the PRC numbering a reliable record
- Each order remembers the ordered material's name at the time it was placed, so the record stays readable even if the material is later renamed or removed

### Data Persistence

Materials, Warehouses, Inventory, Products, BOM and Suppliers are stored using XLNT in an Excel workbook. Procurement Orders and Projections are stored separately as plain text (`data/procurement_orders.txt` and `data/projections.txt`), since both need to be updated after every single action rather than only on a manual save.

## Architecture

The application is structured into several classes with clearly separated responsibilities.

WarehouseSystem
│
├── MaterialManager
│   └── Material
│
├── SupplierManager
│   └── Supplier
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
├── ProcurementManager
│   └── ProcurementOrder
│
├── ProjectionManager
│   └── Projection
│
├── DataManager
│
└── WebServer

## Data Structures

Inventory inside each warehouse is represented using a doubly linked list.

Each WarehouseNode stores:

* A pointer to an existing Material
* The inventory quantity
* A pointer to the previous node
* A pointer to the next node

Materials are owned by MaterialManager and warehouses keep pointers to the existing material objects. This avoids duplicating material master data.

### Product and BOM Concept

A Product contains a Bill of Materials consisting of Material IDs and required quantities.

When a product is sold, the system automatically calculates the required quantities of all BOM components and performs the corresponding inventory issues.

Before changing the inventory, the system checks that all required components are available.

### Persistence

Excel (via XLNT) is used as the persistence layer for Materials, Suppliers, Warehouses, Inventory, Products and BOM.

Procurement Orders and Projections use their own plain text files instead, so that every action (an order confirmed, a receipt logged, a projection registered) is saved immediately, without depending on a manual save to Excel.

The runtime system uses C++ objects and data structures either way; the persistence layer is only responsible for saving and restoring that state.

## Author

Ana Maria Jaramillo
