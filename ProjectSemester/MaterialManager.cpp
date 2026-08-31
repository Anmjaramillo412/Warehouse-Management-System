#include "MaterialManager.h"

#include <iostream>

using namespace std;


// ================================================================
// CREATE MATERIAL
// ================================================================

bool MaterialManager::createMaterial(
    const Material& material)
{
    // Check ID format

    if (!Material::isValidID(
        material.getID()))
    {
        return false;
    }


    // Check if ID already exists

    if (findMaterial(
        material.getID()) != nullptr)
    {
        return false;
    }


    // Create independent Material object

    materials.push_back(
        make_unique<Material>(material));


    return true;
}


// ================================================================
// FIND MATERIAL
// ================================================================

Material* MaterialManager::findMaterial(
    const string& id)
{
    for (const auto& material : materials)
    {
        if (material->getID() == id)
        {
            return material.get();
        }
    }

    return nullptr;
}


// ================================================================
// MODIFY MATERIAL
// ================================================================

bool MaterialManager::modifyMaterial(
    const string& id,
    const Material& newData)
{
    Material* material =
        findMaterial(id);


    if (material == nullptr)
    {
        return false;
    }


    // Do not modify the ID here.

    material->setName(
        newData.getName());

    material->setDescription(
        newData.getDescription());

    material->setUoM(
        newData.getUoM());

    material->setCategory(
        newData.getCategory());

    material->setSupplier(
        newData.getSupplier());

    material->setPhotoPath(
        newData.getPhotoPath());

    material->setActive(
        newData.isActive());


    return true;
}


// ================================================================
// DELETE MATERIAL
// ================================================================

bool MaterialManager::deleteMaterial(
    const string& id)
{
    for (auto it = materials.begin();
        it != materials.end();
        ++it)
    {
        if ((*it)->getID() == id)
        {
            materials.erase(it);

            return true;
        }
    }

    return false;
}


// ================================================================
// DISPLAY MATERIALS
// ================================================================

void MaterialManager::displayMaterials() const
{
    if (materials.empty())
    {
        cout << endl;

        cout << "No materials available."
            << endl;

        return;
    }


    cout << endl;

    cout << "========== MATERIALS =========="
        << endl;


    for (const auto& material : materials)
    {
        material->display();

        cout << endl;
    }
}


// ================================================================
// GET MATERIALS
// ================================================================

const vector<unique_ptr<Material>>&
MaterialManager::getMaterials() const
{
    return materials;
}


// ================================================================
// CLEAR
// ================================================================

void MaterialManager::clear()
{
    materials.clear();
}