#pragma once

#include <iostream>
#include <string>

using namespace std;

class Material
{
private:

    string ID;
    string name;
    string description;
    string UoM;
    string category;
    string supplier;
    string photoPath;
    bool active;

public:

    // Constructor
    Material(
        string id = "",
        string n = "",
        string d = "",
        string u = "pcs",
        string c = "",
        string s = "",
        string p = "",
        bool a = true);

    // Destructor
    ~Material();

    // ID validation
    static bool isValidID(const string& id);

    // Getters
    string getID() const;
    string getName() const;
    string getDescription() const;
    string getUoM() const;
    string getCategory() const;
    string getSupplier() const;
    string getPhotoPath() const;
    bool isActive() const;

    // Setters
    void setID(const string& id);
    void setName(const string& n);
    void setDescription(const string& d);
    void setUoM(const string& u);
    void setCategory(const string& c);
    void setSupplier(const string& s);
    void setPhotoPath(const string& p);
    void setActive(bool a);

    // Display
    void display() const;
};