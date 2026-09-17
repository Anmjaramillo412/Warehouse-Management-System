#include "Material.h"

#include <cctype>


// ================================================================
// PATTERN MATCHING HELPER
// ================================================================
// '#' in the pattern means "one digit".
// Any other character must match literally.

namespace
{
    bool matchesPattern(
        const string& value,
        const string& pattern)
    {
        if (value.length() != pattern.length())
        {
            return false;
        }

        for (size_t i = 0; i < pattern.length(); i++)
        {
            if (pattern[i] == '#')
            {
                if (!isdigit(
                    static_cast<unsigned char>(value[i])))
                {
                    return false;
                }
            }
            else
            {
                if (value[i] != pattern[i])
                {
                    return false;
                }
            }
        }

        return true;
    }
}


// ================================================================
// CONSTRUCTOR
// ================================================================

Material::Material(
    string id,
    string n,
    string d,
    string u,
    string c,
    MaterialType t,
    string drawing,
    string manuf,
    string manufPartNumber,
    Supplier* sup,
    string supPartNumber,
    string p,
    bool a)
{
    ID = id;
    name = n;
    description = d;
    UoM = u;
    category = c;
    type = t;
    drawingNumber = drawing;
    manufacturer = manuf;
    manufacturerPartNumber = manufPartNumber;
    supplier = sup;
    supplierPartNumber = supPartNumber;
    additionalSuppliers.clear();
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
    // Supplier is owned by SupplierManager, not by Material.
}


// ================================================================
// ID VALIDATION
// ================================================================

bool Material::isValidID(const string& id)
{
    // Expected formats:
    // ###-######
    // ######-00

    return matchesPattern(id, "###-######")
        || matchesPattern(id, "######-00");
}


// ================================================================
// DRAWING NUMBER VALIDATION
// ================================================================

bool Material::isValidDrawingNumber(const string& drawingNumber)
{
    // Expected formats:
    // ###-ASM-####
    // ###-PAR-####
    // I-BU#-##
    // OWI-BU#-##
    // ###-PCB-####

    return matchesPattern(drawingNumber, "###-ASM-####")
        || matchesPattern(drawingNumber, "###-PAR-####")
        || matchesPattern(drawingNumber, "I-BU#-##")
        || matchesPattern(drawingNumber, "OWI-BU#-##")
        || matchesPattern(drawingNumber, "###-PCB-####");
}


// ================================================================
// REQUIRES DRAWING NUMBER
// ================================================================

bool Material::requiresDrawingNumber(MaterialType t)
{
    return t == MaterialType::DesignPart
        || t == MaterialType::PCB;
}


// ================================================================
// MATERIAL TYPE <-> STRING
// ================================================================

string Material::materialTypeToString(MaterialType t)
{
    switch (t)
    {
    case MaterialType::DesignPart:
        return "Design Part";

    case MaterialType::PCB:
        return "PCB";

    case MaterialType::StandardPart:
    default:
        return "Standard Part";
    }
}


MaterialType Material::materialTypeFromString(const string& value)
{
    if (value == "Design Part")
    {
        return MaterialType::DesignPart;
    }

    if (value == "PCB")
    {
        return MaterialType::PCB;
    }

    return MaterialType::StandardPart;
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


MaterialType Material::getType() const
{
    return type;
}


string Material::getDrawingNumber() const
{
    return drawingNumber;
}


string Material::getManufacturer() const
{
    return manufacturer;
}


string Material::getManufacturerPartNumber() const
{
    return manufacturerPartNumber;
}


Supplier* Material::getSupplier() const
{
    return supplier;
}


string Material::getSupplierPartNumber() const
{
    return supplierPartNumber;
}


const vector<MaterialSupplierLink>&
Material::getAdditionalSuppliers() const
{
    return additionalSuppliers;
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


void Material::setType(MaterialType t)
{
    type = t;
}


void Material::setDrawingNumber(const string& drawing)
{
    drawingNumber = drawing;
}


void Material::setManufacturer(const string& manuf)
{
    manufacturer = manuf;
}


void Material::setManufacturerPartNumber(const string& manufPartNumber)
{
    manufacturerPartNumber = manufPartNumber;
}


void Material::setSupplier(Supplier* sup)
{
    supplier = sup;
}


void Material::setSupplierPartNumber(const string& supPartNumber)
{
    supplierPartNumber = supPartNumber;
}


void Material::setAdditionalSuppliers(
    const vector<MaterialSupplierLink>& links)
{
    additionalSuppliers = links;
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

    cout << "ID:                " << ID << endl;
    cout << "Name:              " << name << endl;
    cout << "Description:       " << description << endl;
    cout << "UoM:               " << UoM << endl;
    cout << "Category:          " << category << endl;

    cout << "Type:              "
        << materialTypeToString(type)
        << endl;

    cout << "Drawing Number:    " << drawingNumber << endl;
    cout << "Manufacturer:      " << manufacturer << endl;

    cout << "Manufacturer P/N:  "
        << manufacturerPartNumber
        << endl;

    cout << "Supplier:          "
        << (supplier != nullptr
            ? supplier->getName()
            : "N/A")
        << endl;

    cout << "Supplier P/N:      " << supplierPartNumber << endl;

    cout << "Photo:             " << photoPath << endl;

    cout << "Active:            "
        << (active ? "Yes" : "No")
        << endl;

    cout << "----------------------------------------"
        << endl;
}
