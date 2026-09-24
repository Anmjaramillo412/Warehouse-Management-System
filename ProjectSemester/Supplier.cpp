#include "Supplier.h"

#include <sstream>


// ================================================================
// CONSTRUCTOR
// ================================================================

Supplier::Supplier(
    string n,
    string addr,
    string c,
    string cName,
    string cEmail,
    string web,
    vector<string> ordering,
    string payment,
    int leadTime,
    bool internal)
{
    name = n;
    address = addr;
    country = c;
    contactName = cName;
    contactEmail = cEmail;
    website = web;
    orderingMethods = ordering;
    paymentMethod = payment;
    leadTimeWeeks = leadTime;
    isInternal = internal;
}


// ================================================================
// DESTRUCTOR
// ================================================================

Supplier::~Supplier()
{
}


// ================================================================
// GETTERS
// ================================================================

string Supplier::getName() const
{
    return name;
}


string Supplier::getAddress() const
{
    return address;
}


string Supplier::getCountry() const
{
    return country;
}


string Supplier::getContactName() const
{
    return contactName;
}


string Supplier::getContactEmail() const
{
    return contactEmail;
}


string Supplier::getWebsite() const
{
    return website;
}


const vector<string>& Supplier::getOrderingMethods() const
{
    return orderingMethods;
}


string Supplier::getPaymentMethod() const
{
    return paymentMethod;
}


int Supplier::getLeadTimeWeeks() const
{
    return leadTimeWeeks;
}


bool Supplier::getIsInternal() const
{
    return isInternal;
}


// ================================================================
// SETTERS
// ================================================================

void Supplier::setName(const string& n)
{
    name = n;
}


void Supplier::setAddress(const string& addr)
{
    address = addr;
}


void Supplier::setCountry(const string& c)
{
    country = c;
}


void Supplier::setContactName(const string& cName)
{
    contactName = cName;
}


void Supplier::setContactEmail(const string& cEmail)
{
    contactEmail = cEmail;
}


void Supplier::setWebsite(const string& web)
{
    website = web;
}


void Supplier::setOrderingMethods(const vector<string>& ordering)
{
    orderingMethods = ordering;
}


void Supplier::setPaymentMethod(const string& payment)
{
    paymentMethod = payment;
}


void Supplier::setLeadTimeWeeks(int leadTime)
{
    leadTimeWeeks = leadTime;
}


void Supplier::setIsInternal(bool internal)
{
    isInternal = internal;
}


// ================================================================
// ORDERING METHODS <-> STRING
// ================================================================

string Supplier::orderingMethodsToString(
    const vector<string>& methods)
{
    string result = "";

    for (size_t i = 0; i < methods.size(); i++)
    {
        if (i > 0)
        {
            result += ";";
        }

        result += methods[i];
    }

    return result;
}


vector<string> Supplier::orderingMethodsFromString(
    const string& value)
{
    vector<string> result;

    stringstream stream(value);

    string token;

    while (getline(stream, token, ';'))
    {
        if (!token.empty())
        {
            result.push_back(token);
        }
    }

    return result;
}


// ================================================================
// DISPLAY
// ================================================================

void Supplier::display() const
{
    cout << endl;

    cout << "----------------------------------------"
        << endl;

    cout << "Name:           " << name << endl;
    cout << "Address:        " << address << endl;
    cout << "Country:        " << country << endl;
    cout << "Contact Name:   " << contactName << endl;
    cout << "Contact Email:  " << contactEmail << endl;
    cout << "Website:        " << website << endl;

    cout << "Ordering:       "
        << orderingMethodsToString(orderingMethods)
        << endl;

    cout << "Payment:        " << paymentMethod << endl;
    cout << "Lead Time:      " << leadTimeWeeks << " week(s)" << endl;
    cout << "Internal:       " << (isInternal ? "Yes" : "No") << endl;

    cout << "----------------------------------------"
        << endl;
}
