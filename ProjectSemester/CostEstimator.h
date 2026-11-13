#pragma once

// ================================================================
// COST ESTIMATOR (header-only)
// ----------------------------------------------------------------
// Decides which price a Material "costs" - for a quantity and,
// optionally, on a past date - from the two sources that exist:
//
//   Purchase Invoices  - what was actually paid (landed price, with
//                        customs / freight already spread over the
//                        unit prices)
//   Quotations         - what a Supplier offered, with price tiers by
//                        quantity (never includes customs / freight)
//
// RULE: the most recent record BY DATE governs.
//   - The invoice is the same age or newer than the quotation (or
//     there is no quotation): the invoice price is the cost (real).
//     The quotation's tiers are still reported as information, so
//     the price for other quantities can be seen - they never
//     replace the real cost.
//   - The quotation is newer (or there is no invoice): the cost is
//     the quotation's tier price for the quantity (ESTIMATED), plus
//     - only if an invoice of that Material registered customs /
//     freight - that invoice's customs / freight per unit, as a
//     separate ESTIMATED increment. A Material that never had those
//     costs gets nothing added.
//
// Which Supplier's quotation: the Material's primary Supplier; if it
// has none in force, the cheapest one among the additional Suppliers.
//
// With asOfDate (YYYY-MM-DD) only records dated on or before that
// day count (price reconstruction). Without it, everything counts.
// Expired quotations are still used (they are only flagged).
// ================================================================

#include <string>
#include <vector>
#include <cmath>

#include "PurchaseManager.h"
#include "QuotationManager.h"
#include "MaterialManager.h"
#include "ProductManager.h"

using namespace std;


struct MaterialCostEstimate
{
    // "Invoice", "Quotation" or "None" (no price known)
    string source = "None";

    // The cost used (EUR per unit). For a quotation: base + increment.
    double unitPriceEUR = 0.0;

    bool estimated = false;

    // Reference and date of the governing record
    string sourceRef;
    string sourceDate;
    string supplierName;

    // --- quotation governs ---------------------------------------
    double basePriceEUR = 0.0;
    double importIncrementEUR = 0.0;
    bool hasImportIncrement = false;
    string incrementRef;          // invoice the increment comes from
    bool belowFirstTier = false;  // quantity under the first tier
    bool belowMoq = false;        // quantity under the minimum order
    bool quotationExpired = false;
    double tierMinQuantity = 0.0; // start quantity of the tier used
    bool usedUsualTier = false;   // priced at the "usual purchase" tier
    bool usualMissing = false;    // usual basis asked, none marked

    // --- invoice governs, quotation known (information only) -----
    bool hasQuoteInfo = false;
    string quoteRef;
    string quoteSupplier;
    string quoteDate;
    double quotePriceEUR = 0.0;
    bool quoteExpired = false;
    bool quoteBelowFirstTier = false;
    double quoteTierMinQuantity = 0.0;

    // --- quotation governs, an older invoice exists --------------
    bool hasLastInvoice = false;
    string lastInvoiceRef;
    string lastInvoiceDate;
    double lastInvoicePriceEUR = 0.0;
};


class CostEstimator
{
private:

    struct InvoicePick
    {
        bool found = false;
        string invoiceID;
        string date;
        double landedPriceEUR = 0.0;
    };

    // Latest invoice line of the Material on or before asOfDate
    static InvoicePick latestInvoice(
        const PurchaseManager& purchase,
        const string& materialID,
        const string& asOfDate,
        bool onlyWithImportCosts,
        double* importPerUnitEUR)
    {
        InvoicePick best;

        for (const auto& invoice : purchase.getInvoices())
        {
            if (!asOfDate.empty() && invoice->getDate() > asOfDate)
            {
                continue;
            }

            const auto& lines = invoice->getLines();

            for (size_t i = 0; i < lines.size(); i++)
            {
                if (lines[i].materialID != materialID)
                {
                    continue;
                }

                double perUnit = 0.0;

                if (lines[i].receivedQuantity > 0.0)
                {
                    perUnit =
                        invoice->getLineAllocatedCost(i) /
                        lines[i].receivedQuantity *
                        invoice->getExchangeRate();
                }

                if (onlyWithImportCosts && !(perUnit > 0.000001))
                {
                    continue;
                }

                bool newer =
                    !best.found ||
                    invoice->getDate() > best.date ||
                    (invoice->getDate() == best.date &&
                        invoice->getID() >= best.invoiceID);

                if (newer)
                {
                    best.found = true;
                    best.invoiceID = invoice->getID();
                    best.date = invoice->getDate();
                    best.landedPriceEUR =
                        purchase.getLineLandedUnitPriceEUR(
                            invoice.get(), i);

                    if (importPerUnitEUR != nullptr)
                    {
                        *importPerUnitEUR = perUnit;
                    }
                }
            }
        }

        return best;
    }

    // The quotation line in force for this Material: the primary
    // Supplier's, otherwise the cheapest of the additional Suppliers
    static QuotationHit pickQuotation(
        const QuotationManager& quotations,
        const Material* material,
        double quantity,
        const string& asOfDate)
    {
        QuotationHit none;

        if (material == nullptr)
        {
            return none;
        }

        if (material->getSupplier() != nullptr)
        {
            QuotationHit hit =
                quotations.findInForce(
                    material->getID(),
                    material->getSupplier()->getName(),
                    asOfDate);

            if (hit.found())
            {
                return hit;
            }
        }

        QuotationHit cheapest;

        double cheapestPrice = 0.0;

        for (const auto& link : material->getAdditionalSuppliers())
        {
            if (link.supplier == nullptr)
            {
                continue;
            }

            QuotationHit hit =
                quotations.findInForce(
                    material->getID(),
                    link.supplier->getName(),
                    asOfDate);

            if (!hit.found())
            {
                continue;
            }

            const QuotationTier& tier =
                QuotationManager::tierFor(
                    *hit.line, effectiveQuantity(*hit.line, quantity));

            double price =
                QuotationManager::unitPriceEUR(*hit.quotation, tier);

            if (!cheapest.found() || price < cheapestPrice)
            {
                cheapest = hit;

                cheapestPrice = price;
            }
        }

        return cheapest;
    }

    // No quantity given: the usual purchase tier (or the first one)
    static double effectiveQuantity(
        const QuotationLine& line,
        double quantity)
    {
        return quantity > 0.0 ?
            quantity : QuotationManager::referenceQuantity(line);
    }

public:

    static MaterialCostEstimate estimate(
        const string& materialID,
        double quantity,
        const string& asOfDate,
        const PurchaseManager& purchase,
        const QuotationManager& quotations,
        const MaterialManager& materials,
        bool useUsualTier = false)
    {
        MaterialCostEstimate result;

        const Material* material =
            const_cast<MaterialManager&>(materials)
                .findMaterial(materialID);

        double importPerUnit = 0.0;

        InvoicePick invoice =
            latestInvoice(purchase, materialID, asOfDate,
                false, nullptr);

        QuotationHit quote =
            pickQuotation(quotations, material, quantity, asOfDate);

        bool invoiceGoverns =
            invoice.found &&
            (!quote.found() ||
                invoice.date >= quote.quotation->issueDate);

        // ---- the quotation, evaluated at the quantity ------------
        double quotePriceEUR = 0.0;
        bool quoteBelowFirst = false;
        bool quoteBelowMoq = false;
        double quoteTierMin = 0.0;

        bool usedUsual = false;

        bool usualMissing = false;

        if (quote.found())
        {
            double qty = effectiveQuantity(*quote.line, quantity);

            // Reference price: the tier usually bought, instead of
            // the quantity this particular calculation needs
            if (useUsualTier)
            {
                if (quote.line->usualQuantity > 0.0)
                {
                    qty = quote.line->usualQuantity;

                    usedUsual = true;
                }
                else
                {
                    usualMissing = true;
                }
            }

            const QuotationTier& tier =
                QuotationManager::tierFor(
                    *quote.line, qty, &quoteBelowFirst);

            quotePriceEUR =
                QuotationManager::unitPriceEUR(*quote.quotation, tier);

            quoteTierMin = tier.minQuantity;

            quoteBelowMoq =
                !usedUsual &&
                quantity > 0.0 && quote.line->moq > 0.0 &&
                quantity < quote.line->moq;
        }

        if (invoiceGoverns)
        {
            result.source = "Invoice";
            result.unitPriceEUR = invoice.landedPriceEUR;
            result.estimated = false;
            result.sourceRef = invoice.invoiceID;
            result.sourceDate = invoice.date;

            if (quote.found())
            {
                result.hasQuoteInfo = true;
                result.quoteRef = quote.quotation->id;
                result.quoteSupplier = quote.quotation->supplierName;
                result.quoteDate = quote.quotation->issueDate;
                result.quotePriceEUR = quotePriceEUR;
                result.quoteExpired =
                    quotations.status(*quote.quotation) == "Expired";
                result.quoteBelowFirstTier = quoteBelowFirst;
                result.quoteTierMinQuantity = quoteTierMin;
            }

            return result;
        }

        if (!quote.found())
        {
            return result;     // source "None"
        }

        result.source = "Quotation";
        result.estimated = true;
        result.sourceRef = quote.quotation->id;
        result.sourceDate = quote.quotation->issueDate;
        result.supplierName = quote.quotation->supplierName;
        result.basePriceEUR = quotePriceEUR;
        result.belowFirstTier = quoteBelowFirst;
        result.belowMoq = quoteBelowMoq;
        result.tierMinQuantity = quoteTierMin;
        result.usedUsualTier = usedUsual;
        result.usualMissing = usualMissing;
        result.quotationExpired =
            quotations.status(*quote.quotation) == "Expired";

        // Customs / freight per unit of the latest invoice of this
        // Material that registered any (only invoices up to asOfDate)
        InvoicePick withImport =
            latestInvoice(purchase, materialID, asOfDate,
                true, &importPerUnit);

        if (withImport.found)
        {
            result.hasImportIncrement = true;
            result.importIncrementEUR = importPerUnit;
            result.incrementRef = withImport.invoiceID;
        }

        result.unitPriceEUR =
            result.basePriceEUR + result.importIncrementEUR;

        if (invoice.found)
        {
            result.hasLastInvoice = true;
            result.lastInvoiceRef = invoice.invoiceID;
            result.lastInvoiceDate = invoice.date;
            result.lastInvoicePriceEUR = invoice.landedPriceEUR;
        }

        return result;
    }
};


// ================================================================
// PRODUCT COST WITH ESTIMATES
// ================================================================
// Same result shape as PurchaseManager::computeProductCost(), plus
// where each price came from. heaterQuantity is how many Products
// are going to be produced: every material is priced for its BOM
// quantity times that number (so the right price tier applies), but
// the cost shown stays per ONE Product.

struct EstimatedCostLine
{
    string materialID;
    double quantity = 0.0;          // BOM quantity for one Product
    double totalQuantity = 0.0;     // quantity x heaterQuantity

    bool hasPrice = false;
    double unitPriceEUR = 0.0;
    double lineCostEUR = 0.0;

    // Made by an Internal Supplier (4Tex itself): its cost is already
    // inside other materials, so having no price is not "missing"
    bool internal = false;

    MaterialCostEstimate estimate;
};

struct EstimatedProductCost
{
    string productID;

    vector<EstimatedCostLine> lines;

    double totalCostEUR = 0.0;
    double realCostEUR = 0.0;       // from invoices
    double estimatedCostEUR = 0.0;  // from quotations

    bool complete = true;
    bool hasEstimates = false;
};

static EstimatedProductCost computeEstimatedProductCost(
    const string& productID,
    const vector<BOMItem>& bom,
    double heaterQuantity,
    const string& asOfDate,
    const PurchaseManager& purchase,
    const QuotationManager& quotations,
    const MaterialManager& materials,
    bool useUsualTier = false)
{
    EstimatedProductCost result;

    result.productID = productID;

    if (!(heaterQuantity > 0.0))
    {
        heaterQuantity = 1.0;
    }

    for (const auto& item : bom)
    {
        EstimatedCostLine line;

        line.materialID = item.materialID;
        line.quantity = item.quantity;
        line.totalQuantity = roundQty(item.quantity * heaterQuantity);

        line.estimate =
            CostEstimator::estimate(
                item.materialID, line.totalQuantity, asOfDate,
                purchase, quotations, materials, useUsualTier);

        line.hasPrice = line.estimate.source != "None";
        line.unitPriceEUR = line.estimate.unitPriceEUR;
        line.lineCostEUR = line.unitPriceEUR * item.quantity;

        {
            Material* lineMaterial =
                const_cast<MaterialManager&>(materials)
                    .findMaterial(item.materialID);

            line.internal =
                lineMaterial != nullptr &&
                lineMaterial->hasInternalSupplier();
        }

        if (!line.hasPrice)
        {
            // An internal material without a price does not make the
            // Product incomplete (a quotation / invoice price, if it
            // has one, is still counted like any other)
            if (!line.internal)
            {
                result.complete = false;
            }
        }
        else if (line.estimate.estimated)
        {
            result.hasEstimates = true;

            result.estimatedCostEUR += line.lineCostEUR;
        }
        else
        {
            result.realCostEUR += line.lineCostEUR;
        }

        result.totalCostEUR += line.lineCostEUR;

        result.lines.push_back(line);
    }

    return result;
}
