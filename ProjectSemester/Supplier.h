#pragma once

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Supplier
{
private:

    // The Supplier name is the unique key
    // (there is no separate Supplier ID).

    string name;

    string address;
    string country;

    string contactName;
    string contactEmail;

    string website;

    // Multiple selection: "Online", "Email", "PO"
    vector<string> orderingMethods;

    // Single selection: "Invoice", "Pre-payment", "Online"
    string paymentMethod;

    // Open number of weeks
    int leadTimeWeeks;

public:

    // Constructor
    Supplier(
        string n = "",
        string addr = "",
        string c = "",
        string cName = "",
        string cEmail = "",
        string web = "",
        vector<string> ordering = {},
        string payment = "",
        int leadTime = 0);

    // Destructor
    ~Supplier();

    // Getters
    string getName() const;
    string getAddress() const;
    string getCountry() const;
    string getContactName() const;
    string getContactEmail() const;
    string getWebsite() const;
    const vector<string>& getOrderingMethods() const;
    string getPaymentMethod() const;
    int getLeadTimeWeeks() const;

    // Setters
    void setName(const string& n);
    void setAddress(const string& addr);
    void setCountry(const string& c);
    void setContactName(const string& cName);
    void setContactEmail(const string& cEmail);
    void setWebsite(const string& web);
    void setOrderingMethods(const vector<string>& ordering);
    void setPaymentMethod(const string& payment);
    void setLeadTimeWeeks(int leadTime);

    // Serialization helpers
    // (ordering methods stored as a single ";" separated cell)

    static string orderingMethodsToString(
        const vector<string>& methods);

    static vector<string> orderingMethodsFromString(
        const string& value);

    // Display
    void display() const;
};
