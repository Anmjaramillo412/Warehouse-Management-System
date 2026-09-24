#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Supplier.h"

using namespace std;


// ================================================================
// MATERIAL SUPPLIER LINK
// ================================================================
// One extra (non-primary) Supplier a Material can also be purchased
// from, with its own Supplier Part Number. The Material's "main"
// Supplier stays in the existing supplier/supplierPartNumber fields
// below (unchanged, so every place that already reads
// getSupplier()/getSupplierPartNumber() keeps working) - this is
// only for the additional ones.

struct MaterialSupplierLink
{
    Supplier* supplier;

    string supplierPartNumber;
};


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

    // Only required when type is DesignPart or PCB (same as
    // drawingNumber above, always together) - the revision of that
    // technical drawing. Either a plain number ("1", "2", ...) or
    // exactly two uppercase letters ("AA", "BA", ...).
    string drawingVersion;

    // Only applies when type is StandardPart. A DesignPart/PCB
    // material has no manufacturer, no manufacturer part number, no
    // Supplier and no Supplier part number at all - see
    // requiresDrawingNumber() below, which the same materials also
    // satisfy for these fields being blank instead.
    string manufacturer;
    string manufacturerPartNumber;

    // Suppliers are owned by SupplierManager.
    // Material only keeps a pointer to an existing Supplier,
    // the same way Warehouse keeps pointers to existing Materials.
    Supplier* supplier;

    string supplierPartNumber;

    // Non-primary Suppliers this Material can also be purchased from
    // - see MaterialSupplierLink above. The primary one stays in
    // supplier/supplierPartNumber above.
    vector<MaterialSupplierLink> additionalSuppliers;

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
        string drawVersion = "",
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

    // Drawing version validation - a plain number ("1", "2", ...) or
    // exactly two uppercase letters ("AA", "BA", ...).
    static bool isValidDrawingVersion(const string& drawingVersion);

    // Whether this material type requires a drawing number/version
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
    string getDrawingVersion() const;
    string getManufacturer() const;
    string getManufacturerPartNumber() const;
    Supplier* getSupplier() const;
    string getSupplierPartNumber() const;
    const vector<MaterialSupplierLink>& getAdditionalSuppliers() const;
    string getPhotoPath() const;
    bool isActive() const;

    // True when this Material's primary Supplier is an Internal
    // Supplier (e.g. "4Tex GmbH" - self-manufactured/custom-made,
    // not a real external vendor). Such a Material is internal work
    // rather than a real purchase, so it is skipped by Projections
    // (see ProjectionManager::createProjection()) and refused by
    // Procurement Order creation (see WebServer.cpp).
    bool hasInternalSupplier() const;

    // Setters
    void setID(const string& id);
    void setName(const string& n);
    void setDescription(const string& d);
    void setUoM(const string& u);
    void setCategory(const string& c);
    void setType(MaterialType t);
    void setDrawingNumber(const string& drawing);
    void setDrawingVersion(const string& drawVersion);
    void setManufacturer(const string& manuf);
    void setManufacturerPartNumber(const string& manufPartNumber);
    void setSupplier(Supplier* sup);
    void setSupplierPartNumber(const string& supPartNumber);
    void setAdditionalSuppliers(
        const vector<MaterialSupplierLink>& links);
    void setPhotoPath(const string& p);
    void setActive(bool a);

    // Display
    void display() const;
};
