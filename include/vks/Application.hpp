#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <imgui_impl_vulkan.h>

#include <vks/DebugUtilsMessenger.hpp>
#include <vks/Device.hpp>
#include <vks/ImGui/ImGuiApp.hpp>
#include <vks/Instance.hpp>
#include <vks/SwapChain.hpp>
#include <vks/Window.hpp>
#include <vks/Model.hpp>
#include <vks/RenderObject.hpp>
#include <vks/Material.hpp>
#include <vks/Descriptors.hpp>
#include <vks/Camera.hpp>
#include <vks/AssetManager.hpp>
#include <vks/Render/RenderGraph.hpp>


namespace vks
{
    class Application
    {
    public:
        Application();
        ~Application();

        void run();
        static Application& getInstance() { return *m_app; };

        // --- Getters for the CommandBuffer ---
        std::unordered_map<std::string, RenderObject>& getRenderObjects();
        VkDescriptorSet getCameraDescriptorSet() const { return m_cameraDescriptorSet; }
        const CommandPool& getCommandPool() const { return commandPool; };
        Camera& getCamera() { return camera; }
        Window& getWindow() { return window; };
        Instance& getVulkanInstance() { return instance; };
        Ref<DescriptorPool> getGlobalDescriptorPool() { return m_globalDescriptorPool; };

    private:
        void drawFrame(bool& framebufferResized);
        void drawImGui();

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
        void updateUBOs();

        // Static Application Instance
        inline static Application* m_app = nullptr;
        AssetManager m_assets;

        // --- Core Vulkan Objects ---
        Instance instance;
        DebugUtilsMessenger debugMessenger;
        Window window;
        Device device;
        SwapChain swapChain;
        CommandPool commandPool;
        
        // --- Main Renderer ---
        RenderGraph renderGraph;
        Ref<GeometryPipeline> graphicsPipeline;

        Camera camera;

        // --- New Asset Registries ---
        Ref<vks::DescriptorPool> m_globalDescriptorPool;

        std::unique_ptr<vks::Buffer> m_cameraUboBuffer;
        VkDescriptorSet m_cameraDescriptorSet = VK_NULL_HANDLE;
    };
} // namespace vks
