# Warehouse Management System

## Project Description

This project is a C++-based Warehouse Management System.

The system provides functionality for managing materials, warehouses, inventory movements, products, Bills of Materials (BOMs), and persistent data storage.

The application combines an object-oriented C++ backend with a web-based user interface.

## Table of Contents

- [Technologies](#technologies)
- [Getting Started](#getting-started)
- [Main Features](#main-features)
  - [Material Management](#material-management)
  - [Supplier Management](#supplier-management)
  - [Warehouse Management](#warehouse-management)
  - [Inventory Management](#inventory-management)
  - [Product Management](#product-management)
  - [Projection Management](#projection-management)
  - [Procurement Management](#procurement-management)
  - [Purchase Management](#purchase-management)
  - [Data Management](#data-management)
  - [Data Persistence](#data-persistence)
- [Architecture](#architecture)
- [Data Structures](#data-structures)
- [Author](#author)

## Technologies

- C++
- Crow
- XLNT
- HTML
- CSS
- JavaScript
- Microsoft Excel

## Getting Started

Requirements: Visual Studio with the "Desktop development with C++" workload, and [vcpkg](https://vcpkg.io) cloned and bootstrapped at `C:\vcpkg` (the project's **Debug | x64** configuration points there directly).

1. Install the two dependencies with vcpkg:
   ```
   C:\vcpkg\vcpkg install crow xlnt --triplet x64-windows
   ```
2. Open `ProjectSemester/ProjectSemester.vcxproj` in Visual Studio, keep the configuration on **Debug | x64** (the only one set up with vcpkg's paths), and build.
3. Run the project. It starts a Crow web server on port `18080` and keeps running until stopped from the dashboard's **Exit** button (see Data Management below) or the console window.
4. Open `http://localhost:18080` in a browser to use the application.

The `data/` folder is created automatically on first run - see Data Persistence below for what it contains and why none of it is tracked by git.

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

- Create Projections (a planned production batch for a Product, with its own BOM requirement and deadline) - the Warehouse is not chosen manually, it is always the Product's own Main Warehouse (set from Modify Product); a Product with no Main Warehouse assigned yet cannot have a Projection created for it
- Open Projections / Archived Projections views
- Only materials with an actual shortfall are listed as lines on a Projection - not the whole BOM - but that list is not frozen forever at creation: every time an open Projection is read, its Product's full BOM is re-checked and any material that has newly become short (stock consumed by another Projection, a Sale, any Warehouse movement since this Projection was made) is added automatically. A line already on the list is never removed, so its order-registration history stays intact
- "Qty to Order" per material is calculated in delivery-date order, not creation order: each Projection's own number is the combined requirement of itself plus every OTHER open Projection in that Warehouse that needs the same material and is due AT OR BEFORE its own deadline, minus that Warehouse's actual current stock - never a Projection due later. A Projection with no deadline at all is treated as the latest, least-urgent one possible for this purpose only. This means a Projection due sooner keeps its own numbers unaffected by a Projection due later, while a Projection due later correctly accounts for everything already due ahead of it - so two Projections competing for the same scarce material never each get told to order the full shortfall on their own, and a later-due plan can never retroactively change an earlier-due one's. This combined requirement is always read live from each qualifying open Projection's own Product BOM, never from what each Projection happens to have already listed as a line - so even when no single open Projection is short enough on its own to have added a material yet, their combined demand (due at or before the Projection asking) still counts correctly against stock and the material appears as soon as the true total crosses it
- "Ordered" shows, purely for reference, how much of that material is already outstanding on Procurement Orders for that Warehouse - it is never netted against "Qty to Order" automatically, so the user decides how much more (if anything) to actually register
- A material line gets at most two chances to be ordered from the Projection view: if a single Procurement Order placed from that Projection already covers the line's full required quantity, the line freezes ("Already ordered") right away; if the first order placed was for less than that, one more order may be registered afterward - after that second attempt the line freezes regardless of whether it was enough. Once frozen it can no longer be selected to register another order from that same Projection - even if an order is later cancelled or the Warehouse's stock changes - and anything still missing has to be ordered manually from New Order instead. The freeze only lifts when the Projection itself is completed (Confirm Production) or deleted
- Register Procurement Orders directly from a Projection's still-open materials, with a one-click "Register Today" shortcut
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

### Purchase Management

Procurement Management (above) places and receives orders; Purchase Management records what they actually cost.

- Purchase Invoices (`INV-######`, own counter) record what a Material actually cost once the supplier's invoice and the customs/freight bill are in hand - the Procurement Order itself carries no price, so price only enters the system here, at receipt time
- One invoice can cover several materials, even from different Procurement Orders, if they arrived in the same shipment; each line ties back to its Procurement Order and receipt, or to a **Manual Price Adjustment** when a Material is priced with no delivery behind it at all (a price known from another source, or registering several materials' initial prices at once)
- Customs cost and freight cost are entered once per invoice and prorated across its lines "by value" (each line's share of the invoice's total material cost), giving every line its own landed unit price, converted to EUR using the invoice's own exchange rate
- **Other Costs**: an invoice can also carry one or more one-time, Supplier-tied charges that are neither customs nor freight - e.g. a machinery setup fee paid once to a contractor. Unlike customs/freight, an Other Cost is not prorated across this invoice's own lines "by value" - it is amortized as a flat per-unit addition to that Supplier's materials' price, with a per-entry choice of scope: amortized only on this invoice (its fixed amount divided by that Supplier's quantity on this invoice alone), or amortized across all purchases to that Supplier (its fixed amount divided by that Supplier's *cumulative* quantity received across every invoice up to and including this one, oldest to newest). In the Supplier-wide case the per-unit addition shrinks on each later invoice as more is bought from that Supplier, without ever changing what an earlier invoice already computed, and without needing to be re-entered on each later invoice - it was entered once, on the invoice that actually carried the one-time charge, and from then on keeps applying automatically to every later invoice for that Supplier too (e.g. a 600 EUR Other Cost on a first 400-unit invoice adds 1.50 EUR/unit there; a second, unrelated 1000-unit invoice from the same Supplier, with no Other Cost entry of its own, still gets 600 / 1400 = 0.43 EUR/unit from that same original entry). The Invoices view shows, per line, the Other Cost addition per unit and its total EUR value on that line, and the Price Received Deliveries / Edit Invoice forms show the matched quantity for each Other Cost row (computed from that Supplier's lines actually on the invoice) so it is never a hidden number
- Material Price History, and the "current price" hints shown elsewhere in the app, are derived on demand from every Purchase Invoice line recorded for that material - editing an invoice's unit cost later (for when the supplier invoice and the customs bill arrive separately) updates history immediately, with nothing to migrate. The recorded price already includes any Other Cost addition on top of the customs/freight landed price
- Product cost (heater cost evaluation) is computed from the current (latest by date) purchase price of every BOM material, multiplied by its BOM quantity

### Data Management

- Auto Save and Load Data toggle (default on): when enabled, every operation that changes Material/Supplier/Warehouse stock/Product data (Goods Receipt/Issue/Transfer, Procurement receiving, creating/deleting a Material, Supplier, Warehouse or Product, selling a Product) saves automatically right after, and data loads automatically when the server starts - instead of relying on the manual "Save Data"/"Load Data" buttons
- Manual Save Data / Load Data
- Documents Folder setting: the root folder (anywhere on disk) that Procurement's Order Confirmation and Lieferschein PDFs are saved under
- Reset PRC / Invoice Numbering (type `RESET` to confirm): deletes every Procurement Order and every Purchase Invoice outright and restarts both the "PRC-######" and "INV-######" counters at 1 - no undo, meant for clearing test data before real use begins
- Exit: saves all data and shuts the web server down from the dashboard itself, so the application does not need to be stopped from Visual Studio or the console window

### Data Persistence

Materials, Warehouses, Inventory, Products, BOM and Suppliers are stored using XLNT in an Excel workbook, saved only when Auto Save/Load is on or "Save Data" is clicked manually.

Procurement Orders, Procurement document attachments, Projections and Purchase Invoices are each stored separately as plain text (`data/procurement_orders.txt`, `data/procurement_documents.txt`, `data/projections.txt` and `data/purchase_invoices.txt`), saved immediately on every change instead of only on a manual save. A few small settings are stored the same immediate way, each in its own config file: the default exchange rate for Purchase (`data/purchase_config.txt`), Safety Stock units (`data/product_config.txt`), and the Auto Save/Load on/off flag itself (`data/datamanager_config.txt`).

None of `data/` is committed to git - every Material, Supplier, Warehouse stock level, Product, Procurement Order and Purchase Invoice is local to whichever machine/folder the program runs from. Cloning the repository elsewhere gives identical code and an empty `data/` folder - nothing carries over automatically unless the folder is copied across on purpose.

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
├── PurchaseManager
│   └── PurchaseInvoice
│       └── PurchaseInvoiceLine
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

A Material's additional Suppliers are stored as a list embedded directly in the Material record itself (each entry: a Supplier plus its own Supplier Part Number), rather than as a separate table - simpler, but it means the primary Supplier and an additional one cannot currently be swapped, and there is no per-supplier lead time override.

### Product and BOM Concept

A Product contains a Bill of Materials consisting of Material IDs and required quantities.

When a product is sold, the system automatically calculates the required quantities of all BOM components and performs the corresponding inventory issues.

Before changing the inventory, the system checks that all required components are available - except for a BOM component supplied by an Internal Supplier, which is skipped entirely since it is self-manufactured in-house rather than stock drawn from a Warehouse.

### Persistence

Excel (via XLNT) is used as the persistence layer for Materials, Suppliers, Warehouses, Inventory, Products and BOM (the Products sheet includes each Product's optional Main Warehouse ID).

Procurement Orders, Procurement document attachments, Projections and Purchase Invoices use their own plain text files instead, so that every action (an order confirmed, a receipt logged, a projection registered, an invoice recorded) is saved immediately, without depending on a manual save to Excel.

The runtime system uses C++ objects and data structures either way; the persistence layer is only responsible for saving and restoring that state.

## Author

Ana Maria Jaramillo
