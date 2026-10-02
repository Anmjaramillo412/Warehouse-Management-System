#pragma once

#include <vector>
#include <memory>
#include <string>

#include "PurchaseInvoice.h"
#include "ProcurementManager.h"
#include "MovementLogger.h"
#include "Product.h"
#include "MaterialManager.h"

using namespace std;


// ================================================================
// PENDING RECEIPT (not yet priced)
// ================================================================
// One already-received Procurement Order delivery that has no
// Purchase Invoice line pricing it yet - what the "Pending Pricing"
// view in the Purchase module lists for the user to price.

struct PendingPurchaseReceipt
{
    string procurementOrderID;
    string materialID;

    int receiptIndex;

    string receiptDate;
    int receivedQuantity;
    string comment;
};


// ================================================================
// MATERIAL PRICE HISTORY ENTRY
// ================================================================
// One point in a Material's price history - always in EUR, always
// derived from a Purchase Invoice line, never entered directly.

struct MaterialPriceHistoryEntry
{
    string date;

    double unitPriceEUR;

    string originalCurrency;
    double originalUnitPrice;
    double exchangeRateUsed;

    string sourceInvoiceID;
    string sourceProcurementOrderID;
};


// ================================================================
// PRODUCT COST LINE / RESULT
// ================================================================
// The landed-cost rollup of a Product's Bill of Materials, using
// each material's CURRENT unit price (the most recent Purchase
// Invoice line priced for it). A Product whose BOM includes a
// material with no price history yet is marked incomplete, since its
// true cost is not fully known.

struct ProductCostLine
{
    string materialID;
    int quantity;

    bool hasPrice;
    double unitPriceEUR;
    double lineCostEUR;
};

struct ProductCostResult
{
    string productID;

    vector<ProductCostLine> lines;

    double totalCostEUR;

    // false if any BOM material has no price history yet - totalCostEUR
    // still adds up whatever IS known, but should be shown as partial.
    bool complete;
};


// ================================================================
// PURCHASE MANAGER
// ================================================================
// Owns every Purchase Invoice and the module's default exchange
// rate, in its own persistence files (kept separate from Procurement
// and from the Excel workbook, the same way ProcurementManager keeps
// its own file). Purchase only ever READS from ProcurementManager -
// it never modifies a Procurement Order, so Procurement's own data
// and behavior stay untouched by anything done here.

class PurchaseManager
{
private:

    vector<unique_ptr<PurchaseInvoice>> invoices;

    string filename;
    string configFilename;

    int nextNumber;

    double defaultExchangeRate;

    ProcurementManager* procurementManager;
    MovementLogger* movementLogger;

    // Needed to resolve a material's Supplier for the "Other Costs"
    // amortization below (see getLineOtherCostAdditionEUR()). May be
    // nullptr if not wired yet, in which case Other Costs simply
    // contribute nothing - set via setMaterialManager(), the same
    // pattern InventoryManager uses, rather than a constructor
    // argument, since other code already constructs PurchaseManager
    // with its existing argument list.
    MaterialManager* materialManager;

    // Generates the next consecutive ID, e.g. "INV-000001"
    string generateNextID();

    // The name of the Supplier that owns this line's material, or ""
    // if unknown (no MaterialManager wired, material not found, or
    // the material has no Supplier).
    string getLineSupplierName(
        const PurchaseInvoiceLine& line) const;

public:

    // Constructor
    PurchaseManager(
        ProcurementManager* procManager = nullptr,
        MovementLogger* logger = nullptr,
        string file = "data/purchase_invoices.txt",
        string configFile = "data/purchase_config.txt");

    void setProcurementManager(
        ProcurementManager* manager);

    void setMovementLogger(
        MovementLogger* logger);

    void setMaterialManager(
        MaterialManager* manager);

    // ------------------------------------------------------------
    // Pending receipts (not yet priced by any invoice line)
    // ------------------------------------------------------------

    vector<PendingPurchaseReceipt> getPendingReceipts() const;

    // ------------------------------------------------------------
    // Invoices
    // ------------------------------------------------------------

    // Creates a new Purchase Invoice with the given lines. Refuses
    // (returns nullptr) if any line's (procurementOrderID,
    // materialID, receiptIndex) does not exist in Procurement, or is
    // already priced by another invoice, or if the receivedQuantity
    // given does not match the actual receipt - so an invoice can
    // never silently double-price or mis-price a delivery.
    PurchaseInvoice* createInvoice(
        const string& date,
        const string& currency,
        double exchangeRate,
        double customsCost,
        double freightCost,
        const string& comment,
        const vector<PurchaseInvoiceLine>& lines,
        const vector<PurchaseInvoiceOtherCost>& otherCosts,
        string& errorMessage);

    PurchaseInvoice* findInvoice(
        const string& id);

    // Reworks an existing invoice's header (date, currency, exchange
    // rate, customs cost, freight cost, comment) and each line's
    // unitCost - for when the supplier invoice and the customs bill
    // arrive separately and the invoice needs the second half filled
    // in once it does. The deliveries themselves cannot change: lines
    // must be the same ones already on the invoice, in identity
    // (procurementOrderID/materialID/receiptIndex/receivedQuantity) -
    // only unitCost may differ. Every price history entry derived
    // from this invoice updates immediately, since history is
    // computed on demand, not stored separately.
    bool updateInvoice(
        const string& id,
        const string& date,
        const string& currency,
        double exchangeRate,
        double customsCost,
        double freightCost,
        const string& comment,
        const vector<PurchaseInvoiceLine>& lines,
        const vector<PurchaseInvoiceOtherCost>& otherCosts,
        string& errorMessage);

    // Deletes an invoice outright - its priced receipts return to
    // Pending Pricing, and its price history entries disappear. Used
    // to correct a mistake, not for everyday use.
    bool deleteInvoice(
        const string& id);

    const vector<unique_ptr<PurchaseInvoice>>&
        getInvoices() const;

    // ------------------------------------------------------------
    // "Other Costs" amortization (see PurchaseInvoiceOtherCost)
    // ------------------------------------------------------------

    // This line's share of its invoice's Other Costs, in EUR - 0.0 if
    // the line's material has no Supplier, no Other Cost on the
    // invoice names that Supplier, or materialManager was never
    // wired. For an invoice-only Other Cost, the amount is divided by
    // that Supplier's quantity on THIS invoice alone; for a
    // Supplier-wide one, by that Supplier's cumulative quantity
    // across every invoice up to and including this one (oldest to
    // newest by date, tie-broken by ID) - see the .cpp for the exact
    // formula and the worked example that drove it.
    double getLineOtherCostAdditionEUR(
        const PurchaseInvoice* invoice,
        size_t lineIndex) const;

    // getLineUnitPriceEUR() plus getLineOtherCostAdditionEUR() - the
    // actual price recorded into history/current price/product
    // costing (see getPriceHistory() below).
    double getLineLandedUnitPriceEUR(
        const PurchaseInvoice* invoice,
        size_t lineIndex) const;

    // ------------------------------------------------------------
    // Material price history / current price
    // ------------------------------------------------------------

    // Every invoice line pricing this material, oldest first.
    vector<MaterialPriceHistoryEntry> getPriceHistory(
        const string& materialID) const;

    bool hasPriceFor(
        const string& materialID) const;

    // The most recent (by date) price on record, 0.0 if none.
    double getCurrentUnitPriceEUR(
        const string& materialID) const;

    // ------------------------------------------------------------
    // Product costing
    // ------------------------------------------------------------

    ProductCostResult computeProductCost(
        const string& productID,
        const vector<BOMItem>& bom) const;

    // ------------------------------------------------------------
    // Exchange rate configuration
    // ------------------------------------------------------------

    double getDefaultExchangeRate() const;

    bool setDefaultExchangeRate(double rate);

    // ------------------------------------------------------------
    // Persistence
    // ------------------------------------------------------------

    bool save();
    bool load();

    void clear();
};
