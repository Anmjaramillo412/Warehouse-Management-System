#include "PurchaseInvoice.h"

#include <iostream>


// ================================================================
// CONSTRUCTOR
// ================================================================

PurchaseInvoice::PurchaseInvoice(
    string invoiceID,
    string invoiceDate,
    string curr,
    double rate,
    double customs,
    double freight,
    string cmt)
{
    id = invoiceID;
    date = invoiceDate;
    currency = curr;
    exchangeRate = rate;
    customsCost = customs;
    freightCost = freight;
    comment = cmt;
}


// ================================================================
// DESTRUCTOR
// ================================================================

PurchaseInvoice::~PurchaseInvoice()
{
}


// ================================================================
// GETTERS
// ================================================================

string PurchaseInvoice::getID() const
{
    return id;
}


string PurchaseInvoice::getDate() const
{
    return date;
}


string PurchaseInvoice::getCurrency() const
{
    return currency;
}


double PurchaseInvoice::getExchangeRate() const
{
    return exchangeRate;
}


double PurchaseInvoice::getCustomsCost() const
{
    return customsCost;
}


double PurchaseInvoice::getFreightCost() const
{
    return freightCost;
}


string PurchaseInvoice::getComment() const
{
    return comment;
}


const vector<PurchaseInvoiceLine>& PurchaseInvoice::getLines() const
{
    return lines;
}


// ================================================================
// SETTERS
// ================================================================

void PurchaseInvoice::setID(const string& invoiceID)
{
    id = invoiceID;
}


void PurchaseInvoice::setDate(const string& invoiceDate)
{
    date = invoiceDate;
}


void PurchaseInvoice::setCurrency(const string& curr)
{
    currency = curr;
}


void PurchaseInvoice::setExchangeRate(double rate)
{
    exchangeRate = rate;
}


void PurchaseInvoice::setCustomsCost(double customs)
{
    customsCost = customs;
}


void PurchaseInvoice::setFreightCost(double freight)
{
    freightCost = freight;
}


void PurchaseInvoice::setComment(const string& cmt)
{
    comment = cmt;
}


void PurchaseInvoice::addLine(const PurchaseInvoiceLine& line)
{
    lines.push_back(line);
}


void PurchaseInvoice::setLines(const vector<PurchaseInvoiceLine>& newLines)
{
    lines = newLines;
}


// ================================================================
// LANDED COST CALCULATION
// ================================================================

double PurchaseInvoice::getLineMaterialCost(size_t lineIndex) const
{
    if (lineIndex >= lines.size())
    {
        return 0.0;
    }

    return lines[lineIndex].unitCost *
        lines[lineIndex].receivedQuantity;
}


double PurchaseInvoice::getTotalMaterialCost() const
{
    double total = 0.0;

    for (size_t i = 0; i < lines.size(); i++)
    {
        total += getLineMaterialCost(i);
    }

    return total;
}


double PurchaseInvoice::getLineAllocatedCost(size_t lineIndex) const
{
    if (lineIndex >= lines.size())
    {
        return 0.0;
    }

    double totalMaterialCost =
        getTotalMaterialCost();

    if (totalMaterialCost <= 0.0)
    {
        return 0.0;
    }

    double share =
        getLineMaterialCost(lineIndex) / totalMaterialCost;

    return share * (customsCost + freightCost);
}


double PurchaseInvoice::getLineTotalCost(size_t lineIndex) const
{
    if (lineIndex >= lines.size())
    {
        return 0.0;
    }

    return getLineMaterialCost(lineIndex) +
        getLineAllocatedCost(lineIndex);
}


double PurchaseInvoice::getLineUnitPrice(size_t lineIndex) const
{
    if (lineIndex >= lines.size() ||
        lines[lineIndex].receivedQuantity <= 0)
    {
        return 0.0;
    }

    return getLineTotalCost(lineIndex) /
        lines[lineIndex].receivedQuantity;
}


double PurchaseInvoice::getLineUnitPriceEUR(size_t lineIndex) const
{
    return getLineUnitPrice(lineIndex) * exchangeRate;
}


// ================================================================
// DISPLAY
// ================================================================

void PurchaseInvoice::display() const
{
    cout << endl;

    cout << "----------------------------------------"
        << endl;

    cout << "Invoice:          " << id << endl;
    cout << "Date:             " << date << endl;
    cout << "Currency:         " << currency << endl;
    cout << "Exchange Rate:    " << exchangeRate << endl;
    cout << "Customs Cost:     " << customsCost << endl;
    cout << "Freight Cost:     " << freightCost << endl;
    cout << "Comment:          " << comment << endl;
    cout << "Lines:            " << lines.size() << endl;

    cout << "----------------------------------------"
        << endl;
}
