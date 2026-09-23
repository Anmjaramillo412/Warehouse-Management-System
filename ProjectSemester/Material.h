#pragma once

#include <iostream>
#include <string>

#include "Supplier.h"

using namespace std;


// ================================================================
// MATERIAL TYPE
// ================================================================

enum class MaterialType
{
    StandardPart,
    DesignPart,
    PCB
};


class Material
{
private:

    string ID;
    string name;
    string description;
    string UoM;
    string category;

    MaterialType type;

    // Only required when type is DesignPart or PCB.
    // Format: ###-ASM-####, ###-PAR-####,
    //         I-BU#-##, OWI-BU#-##, ###-PCB-####
    string drawingNumber;

    string manufacturer;
    string manufacturerPartNumber;

    // Suppliers are owned by SupplierManager.
    // Material only keeps a pointer to an existing Supplier,
    // the same way Warehouse keeps pointers to existing Materials.
    Supplier* supplier;

    string supplierPartNumber;

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
        MaterialType t = MaterialType::StandardPart,
        string drawing = "",
        string manuf = "",
        string manufPartNumber = "",
        Supplier* sup = nullptr,
        string supPartNumber = "",
        string p = "",
        bool a = true);

    // Destructor
    ~Material();

    // ID validation
    // Accepts ###-###### or ######-00
    static bool isValidID(const string& id);

    // Drawing number validation
    static bool isValidDrawingNumber(const string& drawingNumber);

    // Whether this material type requires a drawing number
    static bool requiresDrawingNumber(MaterialType t);

    // MaterialType <-> string
    static string materialTypeToString(MaterialType t);
    static MaterialType materialTypeFromString(const string& value);

    // Getters
    string getID() const;
    string getName() const;
    string getDescription() const;
    string getUoM() const;
    string getCategory() const;
    MaterialType getType() const;
    string getDrawingNumber() const;
    string getManufacturer() const;
    string getManufacturerPartNumber() const;
    Supplier* getSupplier() const;
    string getSupplierPartNumber() const;
    string getPhotoPath() const;
    bool isActive() const;

    // Setters
    void setID(const string& id);
    void setName(const string& n);
    void setDescription(const string& d);
    void setUoM(const string& u);
    void setCategory(const string& c);
    void setType(MaterialType t);
    void setDrawingNumber(const string& drawing);
    void setManufacturer(const string& manuf);
    void setManufacturerPartNumber(const string& manufPartNumber);
    void setSupplier(Supplier* sup);
    void setSupplierPartNumber(const string& supPartNumber);
    void setPhotoPath(const string& p);
    void setActive(bool a);

    // Display
    void display() const;
};
