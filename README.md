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
  - [Quotation Management](#quotation-management)
  - [User Accounts and Roles](#user-accounts-and-roles)
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
- Display active material master data, with a separate Archived Materials view for materials marked inactive; materials are always listed in Material ID order (also in every Material search dropdown), not in the order they were created
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
- Inventory Count (physical inventory of one Warehouse, several materials at once; Admin and Editor, Viewer read-only):
  - States: **Open** (draft - counted quantities, reasons and comments are saved and can be resumed another day; stock is unchanged), **Applied** (final - stock is set to the counted quantities, no longer editable) and **Cancelled** (no effect). Start and finish dates are recorded. Counts are numbered `CNT-000001` and stored in `data/inventory_counts.txt`
  - While a count is Open its Warehouse is locked: goods receipts, issues, transfers (from or to it), sales, procurement receipts and completing Projections into that Warehouse (and deleting it) are refused with a message until the count is applied or cancelled
  - Per material: system quantity, counted quantity (up to 3 decimals), difference and a reason - Missing material, Surplus material, Damaged / Scrap, Registration error / Other or Inventory adjustment - plus an optional comment (also a general comment per count). Materials with stock 0 can be added and counted; materials without a count stay unchanged
  - Every adjustment is written to the movement history as `INVENTORY ADJUSTMENT` (system, counted, difference, reason, comment), together with `INVENTORY COUNT OPENED / APPLIED / CANCELLED`. The count history can be filtered by status and exported to Excel (CSV)

### Product Management

- BOM quantities accept up to 3 decimals (e.g. 0.25)
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
- Optional Estimated Receipt date, one per Procurement Order (Open and Confirmed Orders, saved as soon as the date is picked; clearing the field removes it): shown inside the card (in the material's row for a single-material order, once above the materials otherwise) and as "ETA" in the collapsed card header. It applies to every line still waiting for goods. Informational only; cannot be before the Order Date
- Confirmed orders cannot be deleted, to keep the PRC numbering a reliable record
- Movement ledger (`data/movement_ledger.csv`, one row per event, always on - independent of the "Log data operations" setting; see Movement Ledger below): every order line created (with its Source: the Projection's "PRJ-######", or "Manual"), every confirmation, every receipt (date, quantity, total received / ordered, pending, status and comment) and every Close (Incomplete) with the quantity that never arrived, each with material, supplier and warehouse. The stock movement of a receipt is still logged separately as a GOODS RECEIPT. Cancellations are deliberately not recorded
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
- **One Supplier per invoice**: every Supplier bills separately, so an invoice covers the deliveries of a single Supplier (the form deselects deliveries of other Suppliers and the server rejects a mixed invoice). An Other Cost therefore never asks for a Supplier: it belongs to the invoice's Supplier, which the server fills in
- **Other Costs**: an invoice can also carry one or more one-time, Supplier-tied charges that are neither customs nor freight - e.g. a machinery setup fee paid once to a contractor. Unlike customs/freight, an Other Cost is not prorated across this invoice's own lines "by value" - it is amortized as a flat per-unit addition to that Supplier's materials' price, with a per-entry choice of scope: amortized only on this invoice (its fixed amount divided by that Supplier's quantity on this invoice alone), or amortized across all purchases to that Supplier (its fixed amount divided by that Supplier's *cumulative* quantity received across every invoice up to and including this one, oldest to newest). In the Supplier-wide case the per-unit addition shrinks on each later invoice as more is bought from that Supplier, without ever changing what an earlier invoice already computed, and without needing to be re-entered on each later invoice - it was entered once, on the invoice that actually carried the one-time charge, and from then on keeps applying automatically to every later invoice for that Supplier too (e.g. a 600 EUR Other Cost on a first 400-unit invoice adds 1.50 EUR/unit there; a second, unrelated 1000-unit invoice from the same Supplier, with no Other Cost entry of its own, still gets 600 / 1400 = 0.43 EUR/unit from that same original entry). The Invoices view shows, per line, the Other Cost addition per unit and its total EUR value on that line, and the Price Received Deliveries / Edit Invoice forms show the matched quantity for each Other Cost row (computed from that Supplier's lines actually on the invoice) so it is never a hidden number
- Material Price History, and the "current price" hints shown elsewhere in the app, are derived on demand from every Purchase Invoice line recorded for that material - editing an invoice's unit cost later (for when the supplier invoice and the customs bill arrive separately) updates history immediately, with nothing to migrate. The recorded price already includes any Other Cost addition on top of the customs/freight landed price
- Product cost (heater cost evaluation) is computed from the current (latest by date) purchase price of every BOM material, multiplied by its BOM quantity

### Quotation Management

Supplier quotations document material prices in advance, so price changes can be followed over time next to the Purchase Invoices (which hold what was actually paid).

- A quotation (`QUO-######`, own counter) belongs to one Supplier and has one line per Material, with the Supplier's own quote number, issue date, optional valid-until date, currency (EUR, or USD with its USD to EUR exchange rate as on invoices), Incoterm, lead time (always in weeks, one value for the whole quotation), comment and an attached PDF (saved in the Documents Folder as `QUO-000001 - quotation.pdf`, like the Procurement documents)
- **Price tiers by quantity** are the core of a line: "from X units, unit price Y" - the quantity bought decides the price. A quantity below the first tier gets the first tier's price and is flagged as below the quoted minimum. Each line also stores the minimum order quantity
- A quotation never includes customs or freight: those only appear once an import happens, on the Purchase Invoice
- **Current and past quotations**: for one Supplier and Material, the quotation with the latest issue date is the current one and replaces the previous one automatically. Older quotations are never deleted or overwritten - they are shown as **Replaced** (with their PDF and tiers) and can be consulted at any time, and a price can be reconstructed for a past date from the quotation that was in force on that date
- Status is calculated: **Valid**, **Expired** (past its valid-until date; it is still used for estimates and shown with a warning) or **Replaced**
- The Pricing module (the former Purchase and Prices modules merged: Pending Pricing, Adjust Material Price, Invoices, Quotations, Material Price History and Settings; Product Cost and Cost by Category moved to the Product Manager) lists the quotations with filters (current, valid, expired, replaced, all; supplier; free text), a detail view with all tiers, create / edit / delete and export to Excel. Viewers can only read
- Creating, changing, deleting a quotation and attaching a PDF are written to the Movement Ledger
- **Link with invoices**: on each invoice line (Price Received Deliveries and Edit Invoice) the quotation it was priced from can be chosen. The unit cost is proposed from the price tier for the received quantity (converted to the invoice currency) and can be corrected. The tier price is stored frozen on the line, and the Invoices view shows the deviation between quoted and invoiced price (before customs / freight, since a quotation has none). A quotation linked to an invoice cannot be deleted
- **Usual purchase quantity**: on each quotation line one tier can be checked as "usual purchase" - the quantity normally bought from that Supplier. When a quotation governs the price and no invoice exists, that tier is the reference price of the material (instead of the tier of the quantity a calculation happens to need, e.g. a small BOM quantity). Product Cost has a "Price basis" selector: usual purchase quantity (default) or quantity needed (BOM x heaters). Materials with no tier marked fall back to the quantity needed and say so. Projections and invoice lines keep using the real quantity
- **Automatic reading of quotation PDFs**: in *New Quotation*, "Fill in from the supplier's PDF" reads a quotation PDF (German or English, with selectable text) and pre-fills the form: supplier, quote number, issue and validity dates, currency, lead time, Incoterm, the materials (matched by the Supplier's article number, including additional Suppliers), their price tiers, minimum order quantity and the usual purchase quantity when a single quantity was quoted; freight, packaging and payment remarks go to the comment. The reading runs in the browser with pdf.js (`web/lib/`, served by the whitelisted `/lib/<file>` route) and the parsing rules are in `web/lib/quotation-reader.js`; the file never leaves the computer. **It never saves anything**: whatever it was not sure about is marked yellow / listed, the quotation is always checked and created by the user, and the PDF is attached automatically on creation. Scans and photos (no selectable text) are not read yet. Each material line also has "Read the price table of a web shop" (RS, Mouser, ...): take a screenshot of the quantity / price table and paste it (Ctrl+V) or choose the image - it is read with OCR in the browser (Tesseract.js with English data, in `web/lib/`, no internet needed; the picture is enlarged and only digits and `, . - + EUR` are accepted) - or paste the copied text of the table. The tiers are filled in and the read text stays visible; prices that rise with the quantity, or a per-pack column ("Pro Packung") that does not agree with the unit price, are marked yellow. A quotation may have no validity date (no warning for it)
- **Which price governs** (`CostEstimator.h`): the most recent record by date. If the last invoice is the same age or newer than the quotation (or there is no quotation), the invoice price is the real cost and the quotation tiers are shown as information only. If the quotation is newer (or there is no invoice), the cost is the quotation's price tier for the quantity, marked **estimated**; if an invoice of that material registered customs / freight, that invoice's customs / freight per unit is added as a separate estimated increment (a material that never had them gets nothing added). The quotation of the material's primary Supplier is used, otherwise the cheapest among its additional Suppliers
- **Where it shows**: Product Cost (with a "Heaters to produce" quantity and a price basis that select each material's price tier, and a "Cost as of" date that rebuilds the cost on a past date from records dated on or before it; the Bill of Materials is always the current one), the Material detail (cost for a typed quantity, tiers of every supplier, a Quotations table and a separate Invoices table with the real prices paid, optional past quotations) and the Projection detail (price and total of each Qty to Order)

### User Accounts and Roles

The web interface requires a login. Every request is checked on the server (not only hidden in the page), according to the role of the logged-in user:

- **Viewer**: read only - can open every list, detail view, history and report, but cannot create, modify, delete, receive, sell, transfer or upload anything (the server answers 403 and the buttons are hidden)
- **Editor**: day-to-day work - everything a Viewer can do, plus creating, modifying, deleting, receiving, ordering, pricing invoices, etc., and manual "Save Data"
- **Admin**: everything an Editor can do, plus user management (Users button in the header: add users, change roles, reset passwords, delete users) and system-level actions (Load Data, Reset PRC / Invoice Numbering, Auto Save/Logging settings, Documents Folder)
- First start: if `data/users.txt` does not exist yet, an `admin` account is created and its random password is printed once on the server console - log in and change it with the "Password" button. Forgot the Admin password? Stop the server, delete `data/users.txt` and start again to get a new one (this removes all accounts)
- Passwords are never stored: only a salted, iterated SHA-256 hash in `data/users.txt` (git-ignored). Minimum 8 characters; 5 wrong logins lock that username for one minute
- Sessions last 12 hours of inactivity, live in server memory (restarting the server logs everybody out) and use an HttpOnly cookie
- There must always be at least one Admin; an Admin cannot delete their own account
- Implemented in the header-only `AuthManager.h` (accounts, hashing, sessions and the permission rules in `AuthManager::decide()`) plus an `AuthMiddleware` that runs before every route

### Data Management

- Auto Save and Load Data toggle (default on): when enabled, every operation that changes Material/Supplier/Warehouse stock/Product data (Goods Receipt/Issue/Transfer, Procurement receiving, creating/deleting a Material, Supplier, Warehouse or Product, selling a Product) saves automatically right after, and data loads automatically when the server starts - instead of relying on the manual "Save Data"/"Load Data" buttons
- Manual Save Data / Load Data
- Documents Folder setting: the root folder (anywhere on disk) that Procurement's Order Confirmation and Lieferschein PDFs are saved under
- Reset PRC / Invoice Numbering (type `RESET` to confirm): deletes every Procurement Order and every Purchase Invoice outright and restarts both the "PRC-######" and "INV-######" counters at 1 - no undo, meant for clearing test data before real use begins
- Exit (available to every role): saves all data and shuts the web server down from the dashboard itself, so the application does not need to be stopped from Visual Studio or the console window

### Movement Ledger

`data/movement_ledger.csv` is the single record of everything that happens in the Warehouse (replaces the old `movement_history.txt`, whose lines are imported automatically on first start). It is a plain CSV that opens in Excel with a double click. Each row has: Timestamp, User, Type, Warehouse, Material ID, Material, Product ID, Product, Qty In, Qty Out, Balance After (stock of that material in that Warehouse right after the movement), Reference (Procurement Order, Projection, Inventory Count, transfer route...), Details and Comment.

- Stock movements: `GOODS RECEIPT`, `GOODS ISSUE`, `TRANSFER OUT` / `TRANSFER IN` (one row per Warehouse), `PRODUCTION CONSUMPTION` (materials issued when a Projection's production is confirmed), `SALE CONSUMPTION` (materials issued by Sell Product), `INVENTORY ADJUSTMENT`
- Product rows: `PRODUCT SALE` for every Sell Product and every confirmed Projection (Reference = "Sell Product" or the PRJ number)
- Events: procurement milestones, projections created / completed / deleted, inventory counts, purchase invoices (Data save/load events only with "Log data operations")
- If the file is open in Excel the rows wait in memory and are written with the next movement
- Material detail and Product Manager show the movements of that material / the sales of that product (`GET /api/movements`)

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
├── QuotationManager
│   └── Quotation
│       └── QuotationLine
│           └── QuotationTier
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

Material quantities (stock, inventory movements, BOM, Projection requirements, Procurement ordered/confirmed/received quantities and Purchase Invoice line quantities) accept up to 3 decimals (e.g. 0.125 kg, 2.5 m). Values are rounded to 3 decimals after every calculation and shown without trailing zeros. Product-level counts (Quantity to Manufacture, Produced Quantity, Quantity Sold, Safety Stock units) remain whole numbers.

When a product is sold, the system automatically calculates the required quantities of all BOM components and performs the corresponding inventory issues.

Before changing the inventory, the system checks that all required components are available - except for a BOM component supplied by an Internal Supplier, which is skipped entirely since it is self-manufactured in-house rather than stock drawn from a Warehouse.

### Persistence

Excel (via XLNT) is used as the persistence layer for Materials, Suppliers, Warehouses, Inventory, Products and BOM (the Products sheet includes each Product's optional Main Warehouse ID).

Procurement Orders, Procurement document attachments, Projections, Purchase Invoices and Quotations (`data/quotations.txt`) use their own plain text files instead, so that every action (an order confirmed, a receipt logged, a projection registered, an invoice recorded) is saved immediately, without depending on a manual save to Excel.

The runtime system uses C++ objects and data structures either way; the persistence layer is only responsible for saving and restoring that state.

## Author

Ana Maria Jaramillo
