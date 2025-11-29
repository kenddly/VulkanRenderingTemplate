#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>

class IAssetRegistry
{
public:
    virtual ~IAssetRegistry() = default;
};

template <typename T>
class AssetRegistry : public IAssetRegistry
{
public:
    void addAsset(const std::string& name, T asset)
    {
        // Using std::move in case T is a heavy object (like Model/Material)
        m_assets.insert_or_assign(name, std::move(asset));
    }

    // Return a REFERENCE (&) so we don't copy heavy objects
    T& getAsset(const std::string& name)
    {
        auto it = m_assets.find(name);
        if (it == m_assets.end())
        {
            throw std::runtime_error("Asset not found: " + name);
        }
        return it->second;
    }

    bool hasAsset(const std::string& name) const
    {
        return m_assets.find(name) != m_assets.end();
    }

    // Helper to get the whole map if needed
    const std::unordered_map<std::string, T>& getAll() const { return m_assets; }
    std::unordered_map<std::string, T>& getAll() { return m_assets; }

private:
    std::unordered_map<std::string, T> m_assets;
};
