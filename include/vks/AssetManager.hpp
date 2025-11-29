#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

#include <vks/AssetRegistry.hpp>

// template based asset manager that has registries for different asset types
class AssetManager
{
public:
    AssetManager() = default;
    ~AssetManager() = default;

    /**
     * @brief Adds an asset to the appropriate registry.
     * Creates the registry automatically if it doesn't exist.
     * Usage: manager.add<vks::Material>("myMat", materialObj);
     */
    template <typename T>
    void add(const std::string& name, T asset)
    {
        // Get or create the registry for type T
        auto* registry = getRegistry<T>();
        registry->addAsset(name, std::move(asset));
    }

    /**
     * @brief Retrieves an asset by name.
     * Usage: vks::Material& mat = manager.get<vks::Material>("myMat");
     */
    template <typename T>
    T& get(const std::string& name)
    {
        auto* registry = getRegistry<T>();
        return registry->getAsset(name);
    }

    /**
     * @brief Checks if an asset exists.
     */
    template <typename T>
    bool contains(const std::string& name)
    {
        // If the registry for T doesn't even exist, the asset definitely doesn't
        std::type_index typeIdx(typeid(T));
        if (m_registries.find(typeIdx) == m_registries.end()) {
            return false;
        }
        return getRegistry<T>()->hasAsset(name);
    }

    /**
     * @brief Returns the raw registry map for a specific type.
     * Useful for iterating over all Materials, all Models, etc.
     */
    template <typename T>
    std::unordered_map<std::string, T>& getMap()
    {
        return getRegistry<T>()->getAll();
    }

private:
    // Internal helper to find or create the specific registry
    template <typename T>
    AssetRegistry<T>* getRegistry()
    {
        // 1. Get the unique Type Index for T
        std::type_index typeIdx(typeid(T));

        // 2. Check if we already have a registry for this type
        if (m_registries.find(typeIdx) == m_registries.end())
        {
            // 3. If not, create a new one and store it as a base pointer
            m_registries[typeIdx] = std::make_unique<AssetRegistry<T>>();
        }

        // 4. Retrieve the base pointer
        IAssetRegistry* basePtr = m_registries[typeIdx].get();

        // 5. Cast it back to the specific derived type
        return static_cast<AssetRegistry<T>*>(basePtr);
    }

    // Storage: Maps a Type Index -> Pointer to Registry Interface
    std::unordered_map<std::type_index, std::unique_ptr<IAssetRegistry>> m_registries;
};