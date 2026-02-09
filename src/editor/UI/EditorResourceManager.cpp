#include <editor/UI/EditorResourceManager.hpp>

#include <core/Log.hpp>
#include <gfx/Texture.hpp>
#include <gfx/Device.hpp>

#include <imgui_impl_vulkan.h>
#include <iostream>


namespace vks {

    EditorResourceManager::EditorResourceManager(const Device& device)
        : m_device(device)
    {
        if (instance!= nullptr)
            LOG_ERROR( "EditorResourceManager instance already exists! This should be a singleton." );

        instance = this; // Set the singleton instance pointer

        // Load standard icons (Ensure these paths exist in your assets folder)
        loadIcon("folder", "assets/icons/folder.png");
        loadIcon("file",   "assets/icons/file.png");
        loadIcon("image",  "assets/icons/image.png");
        loadIcon("font",   "assets/icons/font.png");
        loadIcon("code",   "assets/icons/code.png");
        loadIcon("model",  "assets/icons/model.png");
    }


    void EditorResourceManager::loadIcon(const std::string& keyName, const std::string& filePath) {
        try {
            // Create Vulkan Texture
            auto texture = std::make_shared<Texture>(m_device, filePath);

            // Register with ImGui
            // This creates a Descriptor Set specifically for ImGui
            ImTextureID id = (ImTextureID)ImGui_ImplVulkan_AddTexture(
                texture->getSampler(),
                texture->getImageView(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );

            // Store
            m_textures[keyName] = texture;
            m_icons[keyName] = id;
        }
        catch (const std::exception& e) {
            std::cerr << "[Editor] Failed to load icon '" << keyName << "': " << e.what() << std::endl;
            m_icons[keyName] = NULL; // Fallback to null handle if loading fails
        }
    }

    ImTextureID EditorResourceManager::getIcon(const std::filesystem::path& path) {
        // 1. Is it a Directory?
        if (std::filesystem::is_directory(path)) {
            return m_icons["folder"];
        }

        // 2. check Extension
        std::string ext = path.extension().string();

        // Convert to lowercase for comparison if needed
        // std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga") return getImageIcon(path.string());
        if (ext == ".ttf" || ext == ".otf") return m_icons["font"];
        if (ext == ".cpp" || ext == ".h" || ext == ".hpp" || ext == ".glsl") return m_icons["code"];
        if (ext == ".obj" || ext == ".gltf" || ext == ".fbx") return m_icons["model"];

        // 3. Default File
        return m_icons["file"];
    }

    // Load the image thumbnail and return the ImGui Texture ID for it. This can be used for preview thumbnails in the Asset Browser.
    ImTextureID EditorResourceManager::getImageIcon(const std::string& imagePath)
    {
        if (m_icons.find(imagePath) != m_icons.end()) {
            return m_icons[imagePath];
        }

        try {
            // Load the texture and create an ImGui handle for it
            auto texture = std::make_shared<Texture>(m_device, imagePath);
            ImTextureID id = (ImTextureID)ImGui_ImplVulkan_AddTexture(
                texture->getSampler(),
                texture->getImageView(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );

            // Store it so we can reuse if requested again
            m_textures[imagePath] = texture;
            m_icons[imagePath] = id;

            return id;
        }
        catch (const std::exception& e) {
            std::cerr << "[Editor] Failed to load image thumbnail '" << imagePath << "': " << e.what() << std::endl;
            return NULL; // Fallback to null handle if loading fails
        }
    }
}
