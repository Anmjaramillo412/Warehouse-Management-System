#include "Material.h"

#include <cctype>


// ================================================================
// CONSTRUCTOR
// ================================================================

Material::Material(
    string id,
    string n,
    string d,
    string u,
    string c,
    string s,
    string p,
    bool a)
{
    ID = id;
    name = n;
    description = d;
    UoM = u;
    category = c;
    supplier = s;
    photoPath = p;
    active = a;


    // Default Unit of Measure

    if (UoM.empty())
    {
        UoM = "pcs";
    }
}


// ================================================================
// DESTRUCTOR
// ================================================================

Material::~Material()
{
}


// ================================================================
// ID VALIDATION
// ================================================================

bool Material::isValidID(const string& id)
{
    // Expected format:
    // ###-######

    if (id.length() != 10)
    {
        return false;
    }


    // Position of '-'

    if (id[3] != '-')
    {
        return false;
    }


    // First three characters

    for (int i = 0; i < 3; i++)
    {
        if (!isdigit(
            static_cast<unsigned char>(id[i])))
        {
            return false;
        }
    }


    // Last six characters

    for (int i = 4; i < 10; i++)
    {
        if (!isdigit(
            static_cast<unsigned char>(id[i])))
        {
            return false;
        }
    }


    return true;
}


// ================================================================
// GETTERS
// ================================================================

string Material::getID() const
{
    return ID;
}


string Material::getName() const
{
    return name;
}


string Material::getDescription() const
{
    return description;
}


string Material::getUoM() const
{
    return UoM;
}


string Material::getCategory() const
{
    return category;
}


string Material::getSupplier() const
{
    return supplier;
}


string Material::getPhotoPath() const
{
    return photoPath;
}


bool Material::isActive() const
{
    return active;
}


// ================================================================
// SETTERS
// ================================================================

void Material::setID(const string& id)
{
    ID = id;
}


void Material::setName(const string& n)
{
    name = n;
}


void Material::setDescription(const string& d)
{
    description = d;
}


void Material::setUoM(const string& u)
{
    UoM = u;

    if (UoM.empty())
    {
        UoM = "pcs";
    }
}


void Material::setCategory(const string& c)
{
    category = c;
}


void Material::setSupplier(const string& s)
{
    supplier = s;
}


void Material::setPhotoPath(const string& p)
{
    photoPath = p;
}


void Material::setActive(bool a)
{
    active = a;
}


// ================================================================
// DISPLAY
// ================================================================

void Material::display() const
{
    cout << endl;

    cout << "----------------------------------------"
        << endl;

    cout << "ID:          " << ID << endl;

    cout << "Name:        " << name << endl;

    cout << "Description: " << description << endl;

    cout << "UoM:         " << UoM << endl;

    cout << "Category:    " << category << endl;

    cout << "Supplier:    " << supplier << endl;

    cout << "Photo:       " << photoPath << endl;

    cout << "Active:      "
        << (active ? "Yes" : "No")
        << endl;

    cout << "----------------------------------------"
        << endl;
}