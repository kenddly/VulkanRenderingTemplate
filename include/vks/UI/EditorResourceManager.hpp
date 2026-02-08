#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include <imgui.h>

namespace vks
{
    class Device;
    class Texture;

    class EditorResourceManager
    {
    public:
        explicit EditorResourceManager(const Device& device);
        ~EditorResourceManager() = default;

        /**
         * @brief Returns the appropriate ImGui Texture ID based on the file extension or directory status.
         */
        ImTextureID getIcon(const std::filesystem::path& path);
        ImTextureID getImageIcon(const std::string& imagePath);

        static EditorResourceManager& Instance() { return *instance; }

    private:
        inline static EditorResourceManager* instance = nullptr; // Singleton instance pointer

        const Device& m_device;

        // Keep textures alive so Vulkan ImageViews remain valid
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;

        // Map "name" (e.g., "folder", "cpp") -> ImGui Handle
        std::unordered_map<std::string, ImTextureID> m_icons;

        // Internal helper to load and register a texture
        void loadIcon(const std::string& keyName, const std::string& filePath);
    };
}
