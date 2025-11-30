#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <vks/Basic/BasicRenderPass.hpp>
#include <vks/Basic/BasicCommandBuffers.hpp>
#include <vks/DebugUtilsMessenger.hpp>
#include <vks/Device.hpp>
#include <vks/GraphicsPipeline.hpp>
#include <vks/ImGui/ImGuiApp.hpp>
#include <vks/Instance.hpp>
#include <vks/SwapChain.hpp>
#include <vks/SyncObjects.hpp>
#include <vks/Window.hpp>
#include <vks/Model.hpp>
#include <vks/Material.hpp>
#include <vks/Descriptors.hpp>
#include <vks/Camera.hpp>
#include <vks/AssetManager.hpp>


namespace vks
{


    // A struct to define a "thing" in your scene
    // (This is from your Application.hpp (Updated) file)
    struct RenderObject
    {
        vks::Model* model;
        vks::Material* material;
        glm::mat4 transform;

        uint64_t getSortKey() const
        {
            return material->layer_priority;
        }
    };


    class Application
    {
    public:
        Application();
        ~Application();

        void run();
        static Application& getInstance() { return *m_app; };

        // --- Getters for the CommandBuffer ---
        const std::vector<RenderObject>& getRenderObjects() const { return m_renderObjects; }
        VkDescriptorSet getCameraDescriptorSet() const { return m_cameraDescriptorSet; }
        const CommandPool& getCommandPool() const { return commandPool; };
        const Camera& getCamera() const { return camera; }

    private:
        void drawFrame(bool& framebufferResized);
        void drawImGui();
        void recreateSwapChain(bool& framebufferResized);

        /**
         * @brief Creates all asset registries (pools, models, materials).
         */
        void loadAssets();

        /**
         * @brief Populates the m_renderObjects list.
         */
        void buildScene();

        /**
         * @brief Updates scene data (e.g., camera matrices).
         */
        void updateUBOs(uint32_t currentImage);

        // Static Application Instance
        inline static Application* m_app = nullptr;
        AssetManager m_assets;

        // --- Core Vulkan Objects ---
        // (These are from your original constructor)
        Instance instance;
        DebugUtilsMessenger debugMessenger;
        Window window;
        Device device;
        SwapChain swapChain;
        BasicRenderPass renderPass;
        CommandPool commandPool;
        GraphicsPipeline graphicsPipeline;
        BasicCommandBuffers commandBuffers;
        SyncObjects syncObjects;
        ImGuiApp interface;

        Camera camera;

        int currentFrame = 0;

        // --- New Asset Registries ---
        Ref<vks::DescriptorPool> m_globalDescriptorPool;
        std::map<std::string, vks::Model> m_models;

        std::vector<Material> m_materials;

        // --- New Scene Data ---
        std::vector<RenderObject> m_renderObjects;
        std::unique_ptr<vks::Buffer> m_cameraUboBuffer;
        VkDescriptorSet m_cameraDescriptorSet = VK_NULL_HANDLE;
    };
} // namespace vks
