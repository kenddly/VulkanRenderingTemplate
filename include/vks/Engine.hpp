#pragma once

#include <memory>

#include <vks/Instance.hpp>
#include <vks/DebugUtilsMessenger.hpp>
#include <vks/Window.hpp>
#include <vks/Device.hpp>
#include <vks/SwapChain.hpp>
#include <vks/CommandPool.hpp>
#include <vks/AssetManager.hpp>
#include <vks/Camera.hpp>
#include <vks/Descriptors.hpp>
#include <vks/Scene.hpp>
#include <vks/Render/RenderGraph.hpp>
#include <vks/Buffer.hpp>


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

    private:
        void onInit();
        void drawFrame(bool& framebufferResized);
        void updateCameraUBO();

        // Core
        Instance m_instance;
        DebugUtilsMessenger m_debugMessenger;
        Window m_window;
        Device m_device;
        SwapChain m_swapChain;
        CommandPool m_commandPool;

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
    };
}
