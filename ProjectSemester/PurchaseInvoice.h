#pragma once

#include <string>
#include <vector>

using namespace std;


// ================================================================
// PURCHASE INVOICE LINE
// ================================================================
// Prices ONE already-received Procurement Order line (one specific
// delivery/receipt of one material) against this invoice. A single
// invoice can cover several receipts, even from different
// Procurement Orders, when they arrived together in the same
// shipment/invoice from the supplier.
//
// receiptIndex identifies WHICH receipt of that order+material this
// is (a Procurement Order line can have several partial receipts on
// different dates, each potentially its own invoice) - it is the
// position of that receipt in ProcurementOrder::getReceipts(), which
// never changes once recorded, so it stays a stable reference.
//
// A MANUAL PRICE ADJUSTMENT is the one exception: a direct price
// registration for a material with no Procurement delivery behind it
// (see PurchaseManager::createInvoice()) - e.g. a price known from
// another source, or setting up initial prices for several materials
// at once the way a real supplier invoice would arrive, with its own
// customs/freight. It is marked by receiptIndex -1 and
// procurementOrderID "MANUAL"; receivedQuantity is given directly by
// the user (not tied to any real delivery), so several manual lines
// on one invoice still get customs/freight prorated "by value" across
// them exactly like normal delivery lines do.

struct PurchaseInvoiceLine
{
    string procurementOrderID;
    string materialID;

    int receiptIndex;

    // Cached from the receipt at the time this line was created, so
    // the cost calculation below does not depend on Procurement data
    // still existing/matching later.
    int receivedQuantity;

    // This line's own per-unit cost as stated on the supplier's
    // invoice, in the invoice's currency (see PurchaseInvoice) - the
    // user enters this directly; the line's subtotal (unitCost *
    // receivedQuantity, see PurchaseInvoice::getLineMaterialCost())
    // is what customs and freight get allocated against, prorated
    // across every line of the invoice (see PurchaseInvoice below).
    double unitCost;
};


// ================================================================
// PURCHASE INVOICE
// ================================================================
// One landed-cost calculation: a supplier invoice (or the relevant
// slice of one) covering one or more already-received Procurement
// Order lines, plus the customs (Zoll) and freight/transport cost
// that applies to the whole invoice and must be shared out across
// its lines - proportionally to each line's own material cost
// ("by value"), the standard landed-cost method.
//
// Every material price recorded this way is kept forever as part of
// that material's price history (see PurchaseManager) - an invoice
// is never silently overwritten, only deleted outright if it was a
// mistake (which also removes its price history entries).

class PurchaseInvoice
{
private:

    string id;

    string date;

    // "EUR" or "USD" - the currency the supplier invoice was issued
    // in. Everything is shown to the user in EUR; when currency is
    // "USD", exchangeRate is the USD -> EUR rate used for this
    // specific invoice (kept on the invoice itself, not just in the
    // module's default setting, so a past invoice's math never
    // changes if the default rate is later adjusted).
    string currency;

    double exchangeRate;

    // Customs (Zoll) and freight/transport for the WHOLE invoice, in
    // the invoice's own currency - prorated across every line below
    // by each line's share of the invoice's total material cost.
    double customsCost;
    double freightCost;

    string comment;

    vector<PurchaseInvoiceLine> lines;

public:

    // Constructor
    PurchaseInvoice(
        string invoiceID = "",
        string invoiceDate = "",
        string curr = "EUR",
        double rate = 1.0,
        double customs = 0.0,
        double freight = 0.0,
        string cmt = "");

    // Destructor
    ~PurchaseInvoice();

    // Getters
    string getID() const;
    string getDate() const;
    string getCurrency() const;
    double getExchangeRate() const;
    double getCustomsCost() const;
    double getFreightCost() const;
    string getComment() const;
    const vector<PurchaseInvoiceLine>& getLines() const;

    // Setters
    void setID(const string& invoiceID);
    void setDate(const string& invoiceDate);
    void setCurrency(const string& curr);
    void setExchangeRate(double rate);
    void setCustomsCost(double customs);
    void setFreightCost(double freight);
    void setComment(const string& cmt);

    void addLine(const PurchaseInvoiceLine& line);
    void setLines(const vector<PurchaseInvoiceLine>& newLines);

    // ------------------------------------------------------------
    // Landed cost calculation
    // ------------------------------------------------------------

    // unitCost * receivedQuantity for this line, in the invoice
    // currency, before customs/freight - the user enters unitCost,
    // this is always computed from it, never entered directly.
    double getLineMaterialCost(size_t lineIndex) const;

    // Sum of getLineMaterialCost() over every line (in the invoice
    // currency)
    double getTotalMaterialCost() const;

    // This line's share of customsCost + freightCost, proportional
    // to its own getLineMaterialCost() against getTotalMaterialCost()
    double getLineAllocatedCost(size_t lineIndex) const;

    // getLineMaterialCost() + getLineAllocatedCost(), in the invoice
    // currency
    double getLineTotalCost(size_t lineIndex) const;

    // getLineTotalCost() / receivedQuantity, in the invoice currency -
    // the landed per-unit price, i.e. unitCost plus its share of
    // customs/freight
    double getLineUnitPrice(size_t lineIndex) const;

    // getLineUnitPrice() converted to EUR using exchangeRate
    // (exchangeRate is 1.0 when currency is already "EUR")
    double getLineUnitPriceEUR(size_t lineIndex) const;

    // Display
    void display() const;
};
