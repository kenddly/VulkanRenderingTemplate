#pragma once

#include <memory>

#include <gfx/Instance.hpp>
#include <gfx/DebugUtilsMessenger.hpp>
#include <platform/Window.hpp>
#include <gfx/Device.hpp>
#include <gfx/SwapChain.hpp>
#include <gfx/CommandPool.hpp>
#include <assets/AssetManager.hpp>
#include <scene/Camera.hpp>
#include <gfx/Descriptors.hpp>
#include <scene/Scene.hpp>
#include <render/RenderGraph.hpp>
#include <gfx/Buffer.hpp>
#include <editor/UI/EngineEditor.hpp>


namespace vks
{
    struct EngineConfig
    {
        uint32_t width = 1280;
        uint32_t height = 720;
        const char* appName = "Vulkan Engine";
        const char* engineName = "VKS";
        bool enableValidation = true;
    };

    class Application;

    class Engine
    {
    public:
        Engine(const EngineConfig& config);
        ~Engine();

        // Main entry point
        void run(Application& app);

        // --- Core API ---
        Device& device() { return m_device; }
        Window& window() { return m_window; }
        RenderGraph& renderer() { return m_renderGraph; }
        Scene& scene() { return m_scene; }
        EngineEditor& editor() { return m_editor; }
        AssetManager& assets() { return m_assets; }

        Camera& camera() { return m_camera; }
        Ref<Buffer> cameraBuffer() { return m_cameraUboBuffer; }
        VkDescriptorSet& cameraDescriptorSet() { return m_cameraDescriptorSet; }

        CommandPool& commandPool() { return m_commandPool; }
        Instance& vulkanInstance() { return m_instance; }

        Ref<DescriptorPool> globalDescriptorPool() { return m_globalDescriptorPool; }
        VkDescriptorSet cameraDescriptorSet() const { return m_cameraDescriptorSet; }

        // --- Extension Points ---
        void registerRenderPass(Ref<IRenderPass> pass);
        Ref<DescriptorSetLayout> getDescriptorSetLayout(const std::string& name) const;

        Ref<RenderTarget> getRenderTarget() { return viewportTarget; }

    private:
        void onInit();
        void drawFrame();
        void updateCameraUBO();

        void onImGui();
        void handleRecreate();

        // Core
        Instance m_instance;
        DebugUtilsMessenger m_debugMessenger;
        Window m_window;
        Device m_device;
        CommandPool m_commandPool;
        Ref<SwapChain> m_swapChain;
        Ref<RenderTarget> viewportTarget;

        // Systems
        RenderGraph m_renderGraph;
        AssetManager m_assets;
        Camera m_camera;

        Scene m_scene;

        // Global GPU Resources
        Ref<DescriptorPool> m_globalDescriptorPool;
        std::unordered_map<std::string, Ref<DescriptorSetLayout>> m_descriptorSetLayouts;
        Ref<Buffer> m_cameraUboBuffer;
        VkDescriptorSet m_cameraDescriptorSet = VK_NULL_HANDLE;

        std::vector<Ref<RenderTarget>> viewportRenderTargets;
        bool m_dirtySwapChain = false;
        bool m_dirtyViewport = false;
        VkExtent2D m_newViewportExtent = {1, 1};
        VkExtent2D m_newWindowExtent = {1, 1};

        // Editor Mode (Enables ImGui and other editor features)
        EngineEditor m_editor;
    };
}
