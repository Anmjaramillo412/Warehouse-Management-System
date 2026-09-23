# Architecture Notes — Material Suppliers, Purchase Module & Persistence

Status: mixed — parts of this were implemented differently than originally
planned below. Each section says what actually exists in the code today.
Update this file whenever the design changes, so it never goes stale again.

## Guiding rule

Keep modules independent, each owning its own data and its own persistence
file where practical (the pattern `ProcurementManager` / `ProjectionManager`
/ `PurchaseManager` follow). A module may reference another module's records
by their existing key (materialID, supplierName, procurementOrderID, ...)
but should avoid embedding or duplicating another module's data. Section 1b
below is the one place this codebase currently breaks that rule.

Suppliers are identified by **name** (`string supplierName`), matching how
`SupplierManager::findSupplier(name)` works everywhere in this codebase.
There is no synthetic supplier ID.

---

## 1. Materials domain — `feature/materials` branch

### 1a. Display Materials — two tables — **IMPLEMENTED as planned**

Active materials and Inactive/Archived materials are shown as two separate
views (`Material::isActive()`), exactly as designed. No open items here.

### 1b. Material ↔ Supplier — **IMPLEMENTED, but differently than planned**

The original plan called for a many-to-many relationship via its own
`MaterialSupplierManager` and its own file (`data/material_suppliers.txt`),
so this data would live independently the way Procurement/Purchase do.

What actually shipped instead: a `vector<MaterialSupplierLink>
additionalSuppliers` living directly inside `Material` (`Material.h`), next
to the existing primary `Supplier*`. Each `MaterialSupplierLink` holds a
`Supplier*` and its own `supplierPartNumber`. There is no `isPrimary` flag
or per-supplier `leadTimeWeeks` override — the primary Supplier is still
just the one pointer `Material` already had, and every other linked
Supplier is "additional," with no ranking among them.

Persistence follows `Material` too: `DataManager::save()`/`load()` pack the
whole list into one Excel cell ("Additional Suppliers" column, `Name,Part
Number` pairs joined by `~`), rather than the flat one-row-per-link text
file the original plan called for.

This is simpler and it works, but it means Material↔Supplier data does not
follow the "own module, own file" rule the rest of this document holds
everything else to — it is coupled to `Material`'s own save/load and to the
Excel workbook. Promoting an additional Supplier to primary, or attaching a
lead time to one specific Supplier relationship, are not supported today;
either would need real changes to `Material`, not just a new manager.
Splitting this out into its own `MaterialSupplierManager` is still on the
table if that flexibility becomes worth the migration.

---

## 2. Purchase module — **IMPLEMENTED, with a different design than planned**

The original plan (sections 2a-2c further below, kept for history) proposed
three separate entities: a `ProcurementOrder.unitPrice` field, a
`ShipmentCost` table for customs/freight per shipment, and an append-only
`PurchaseRecord` computed at receipt time. None of those three exist. What
was built instead is a single new module, `PurchaseManager` /
`PurchaseInvoice` (own file, `data/purchase_invoices.txt`, saves
immediately on every change like Procurement/Projection do):

```
PurchaseInvoice
    id                  // "INV-######", own counter
    date, currency ("EUR"/"USD"), exchangeRate
    customsCost         // whole-invoice Zoll, in the invoice's currency
    freightCost         // whole-invoice freight/transport, same currency
    comment
    lines: PurchaseInvoiceLine[]

PurchaseInvoiceLine
    procurementOrderID, materialID, receiptIndex, receivedQuantity
    unitCost            // per-unit cost as stated on the supplier invoice
                         // (the user enters this; the line's material
                         // cost = unitCost * receivedQuantity is always
                         // computed, never entered directly)
```

Landed cost lives on `PurchaseInvoice` itself rather than in a separate
`PurchaseRecord`: `customsCost + freightCost` for the whole invoice is
prorated across its lines "by value" (each line's share of the invoice's
total material cost), giving each line a landed unit price
(`getLineUnitPrice()`), converted to EUR via the invoice's own
`exchangeRate` (`getLineUnitPriceEUR()`). So one invoice can cover several
materials, even from different Procurement Orders, arriving in the same
shipment — which is what `ShipmentCost` was meant to solve, just folded
into the invoice itself instead of a separate table.

A **Manual Price Adjustment** is the other difference from the original
plan: a `PurchaseInvoiceLine` with `receiptIndex -1` and
`procurementOrderID "MANUAL"` prices a Material directly with no
Procurement delivery behind it at all (a price known from another source,
or registering several materials' initial prices at once with their own
Zoll/freight, the way "Adjust Material Price" works today). `receivedQuantity`
on a manual line is whatever the user enters, not fixed at 1 — customs/
freight are prorated across manual and real-delivery lines the same way.

Material price history (`MaterialPriceHistoryEntry`, what "Material Price
History" and "current price" hints read from) is derived on demand from
every `PurchaseInvoiceLine` across every invoice for that material — not
stored separately, so editing an invoice's `unitCost` later (see
`PurchaseManager::updateInvoice()`, for when the supplier invoice and the
customs bill arrive separately) updates history immediately with no
migration step.

Product costing (`PurchaseManager::computeProductCost()`) reads the current
(latest by date) price per BOM material and multiplies by BOM quantity,
same idea as section 2d below proposed — this part matches the original
plan, just sourced from `PurchaseInvoiceLine` history instead of
`PurchaseRecord`.

### `ProcurementOrder.unitPrice` / `supplierName` (2a) — **not implemented**

`ProcurementOrder` still has no price field and no per-order `supplierName`
distinct from the Material's own Supplier. Price only enters the system at
Purchase Invoice time, disconnected from which Supplier the order was
actually placed with. If 4Tex needs to track price *at order time* (before
receipt), or an order placed with a non-primary Supplier, that is still
open — nothing in this document's current design captures it.

---

## 3. Persistence overview — how every module actually saves today

Six modules, three different persistence styles, each chosen module-by-
module rather than as one uniform policy:

- **Excel workbook** (`data/warehouse_data.xlsx`, via `DataManager` +
  XLNT): Material, Supplier, Warehouse (stock), Product/BOM. Saved only
  when `DataManager::save()` runs — either the manual "Save Data" button,
  or automatically (see Auto Save and Load Data below).
- **Own plain-text file, saved immediately on every change**:
  `ProcurementManager` (`data/procurement_orders.txt`), `ProjectionManager`
  (`data/projections.txt`), `PurchaseManager`
  (`data/purchase_invoices.txt`). These never depend on the Excel save.
- **Own small config file, saved immediately when changed**:
  `PurchaseManager`'s default exchange rate (`data/purchase_config.txt`),
  `ProductManager`'s Safety Stock units (`data/product_config.txt`),
  `DataManager`'s own Auto Save and Load Data on/off flag
  (`data/datamanager_config.txt`).

None of `data/` is committed to git (see `.gitignore`) — every one of these
files, and therefore every Material, Supplier, Warehouse stock level,
Product, Procurement Order and Purchase Invoice, is local to whichever
machine/folder the program runs from. Cloning the repository elsewhere
gives identical code and an empty `data/` — nothing to migrate, but also
nothing carries over automatically; each environment starts from zero
unless someone copies the `data/` folder across on purpose.

### Auto Save and Load Data (added after this document's original version)

`DataManager` now has an on/off setting (Data Management → Settings,
default **on**): when enabled, every operation that changes
Material/Supplier/Warehouse stock/Product data (Goods Receipt/Issue/
Transfer, Procurement receiving, creating/deleting a Material, Supplier,
Warehouse or Product, selling a Product) triggers `DataManager::save()`
right after, the same as clicking "Save Data" — see `autoSaveIfEnabled()`
in `WebServer.cpp`, called from every route that touches that data. The
same setting also triggers a `DataManager::load()` automatically when the
server starts (`WebServer::run()`), instead of waiting for someone to click
"Load Data". This closed a real gap: Procurement/Projection/Purchase always
saved themselves immediately, but Warehouse stock changes (Goods Receipt,
Issue, Transfer, selling a Product) did not, and were only safe once
someone remembered to hit "Save Data" before closing the program.

### Reset PRC / Invoice numbering

Data Management → Settings also has a "Reset PRC / Invoice Numbering"
action (type `RESET` to confirm): deletes every Procurement Order and every
Purchase Invoice outright (`ProcurementManager::clear()` +
`PurchaseManager::clear()`, then `save()`), restarting both "PRC-######"
and "INV-######" counters at 1. No undo — meant for clearing test data
before real use begins, not for everyday use.

---

## Open items / decisions still not made

- Whether Material↔Supplier (1b) is ever worth splitting into its own
  `MaterialSupplierManager`, given it currently works but is coupled to
  `Material`'s own save/load.
- Whether `ProcurementOrder` needs its own `supplierName`/price fields
  (2a) for tracking price *at order time*, separate from Purchase Invoice
  pricing at receipt time.
- Costing method for "current" material cost: today it is simply the
  latest (most recent by date) `PurchaseInvoiceLine` price for that
  material. Average of last N purchases, or FIFO, were never decided
  against this — Ana to confirm this still matches how 4Tex wants heater
  costs evaluated.
- Whether any of the plain-text/config files above are worth migrating to
  something more structured (SQLite, etc.) now that there are six of them
  plus the Excel workbook — raised in conversation, not yet decided.
