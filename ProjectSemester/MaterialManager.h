#pragma once

#include <vector>
#include <memory>

#include "Material.h"

using namespace std;

class MaterialManager
{
private:

    vector<unique_ptr<Material>> materials;

public:

    // Create
    bool createMaterial(
        const Material& material);

    // Search
    Material* findMaterial(
        const string& id);

    // Modify
    bool modifyMaterial(
        const string& id,
        const Material& newData);

    // Delete
    bool deleteMaterial(
        const string& id);

    // Display
    void displayMaterials() const;

    // Access
    const vector<unique_ptr<Material>>&
        getMaterials() const;

    // Clear all materials
    void clear();
};