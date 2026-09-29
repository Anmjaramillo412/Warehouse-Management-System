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
- Material Detail dashboard for a single material, reachable from any materials table, with a "Modify Material" shortcut straight into the edit form for that same material
- Display Materials / Archived Materials table: fixed-width columns sized to fit the page without a horizontal scroll bar, a two-line "Drawing Version" header to keep that column narrow, and a header row that stays visible ("sticky") while scrolling down a long list
- Material ID validation using the format `###-######` or `######-00`
- Material image management
- Two Material Types:
  - **Standard Part**: a commercially available part, identified by Manufacturer / Manufacturer Part Number and/or a Supplier Part Number
  - **Design Part / PCB**: a custom-made part identified by its technical drawing (Drawing Number + Drawing Version) instead of a commercial part number - has no Manufacturer, Manufacturer Part Number or Supplier Part Number, but can still have a primary Supplier (name only) and any number of Additional Suppliers, the same as a Standard Part
- One primary Supplier per material, plus any number of additional Suppliers, each with its own Supplier Part Number (Design Part / PCB materials only record the Supplier's name, never a Supplier Part Number, since no commercial part number exists)

### Supplier Management

- Create, search, modify and delete suppliers
- Supplier details: address, country, contact, website, ordering methods, payment method, lead time
- Linked to Materials (as the primary Supplier or as an additional one) and used to group Procurement Orders by supplier
- **Internal Supplier** flag (e.g. the company's own name), marking a Supplier as in-house work rather than a real external vendor. A Material with an Internal Supplier as its primary Supplier is self-manufactured, not purchased, and is treated accordingly everywhere stock is expected to come from a purchase:
  - Skipped entirely from a Projection's BOM shortfall calculation (never shows up as something to order)
  - Blocked from being added to a Procurement Order (single or batch), with a clear error message
  - Skipped from Sell Product's stock check and deduction, so a Product whose BOM includes an in-house part can still be sold without that part ever needing to sit in a Warehouse

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

- Create products, searching for each BOM component by ID or name instead of typing it in by hand
- Define Bills of Materials
- Modify products, with the Product ID itself also picked from a searchable list instead of typed in
- Display products, including each BOM component's Drawing Number (for Design Part / PCB materials) in its own column, and a header row that stays visible while scrolling
- **Main Warehouse** per product (optional, set from Create Product or Modify Product): once set, Display Products' "Stock by Warehouse" column shows only that one Warehouse's stock for the product instead of listing every Warehouse. A product without one keeps the original behavior of showing every Warehouse
- Delete products
- Sell products using automatic BOM-based material consumption (BOM materials from an Internal Supplier are excluded, see Supplier Management above)

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
- Document attachments per whole "PRC-######" order (shared across every material line of that order):
  - **Order Confirmation**: a single PDF; uploading again replaces the previous one
  - **Lieferschein / Delivery Note**: any number of PDFs (one per shipment, for a partial delivery); every upload adds another one instead of replacing, saved as `PRC-###### - DeliveryNote1.pdf`, `PRC-###### - DeliveryNote2.pdf`, and so on
  - Both are saved under a single "Documents Folder" the user chooses once, from Data Management Settings, and can be viewed again directly from the order's card

### Data Management

- Auto Save and Load Data toggle: when on, Material/Supplier/Warehouse/Product data loads automatically at startup and saves automatically after every change, instead of relying on the manual "Save Data" button
- Manual Save Data / Load Data
- Documents Folder setting: the root folder (anywhere on disk) that Procurement's Order Confirmation and Lieferschein PDFs are saved under
- Exit: saves all data and shuts the web server down from the dashboard itself, so the application does not need to be stopped from Visual Studio or the console window

### Data Persistence

Materials, Warehouses, Inventory, Products, BOM and Suppliers are stored using XLNT in an Excel workbook. Procurement Orders, Procurement document attachments, and Projections are stored separately as plain text (`data/procurement_orders.txt`, `data/procurement_documents.txt` and `data/projections.txt`), since all three need to be updated after every single action rather than only on a manual save.

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

Before changing the inventory, the system checks that all required components are available - except for a BOM component supplied by an Internal Supplier, which is skipped entirely since it is self-manufactured in-house rather than stock drawn from a Warehouse.

### Persistence

Excel (via XLNT) is used as the persistence layer for Materials, Suppliers, Warehouses, Inventory, Products and BOM (the Products sheet includes each Product's optional Main Warehouse ID).

Procurement Orders, Procurement document attachments, and Projections use their own plain text files instead, so that every action (an order confirmed, a receipt logged, a projection registered) is saved immediately, without depending on a manual save to Excel.

The runtime system uses C++ objects and data structures either way; the persistence layer is only responsible for saving and restoring that state.

## Author

Ana Maria Jaramillo
