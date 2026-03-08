#include <app/Engine.hpp>
#include <app/Application.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <app/EngineContext.hpp>
#include <render/passes/ImGuiRenderPass.hpp>
#include <render/passes/GeometryPass.hpp>
#include <render/passes/UIPass.hpp>

#include "core/Log.hpp"
#include "platform/events/EventManager.hpp"
#include "platform/events/Events.hpp"

namespace vks
{
    Engine::Engine(const EngineConfig& config)
        : m_instance(config.appName, config.engineName, config.enableValidation),
          m_debugMessenger(m_instance),
          m_newWindowExtent({config.width, config.height}),
          m_window({config.width, config.height}, config.appName, m_instance),
          m_device(m_instance, m_window, Instance::DeviceExtensions),
          m_swapChain(std::make_shared<SwapChain>(m_device, m_window)),
          m_commandPool(m_device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
          m_renderGraph(m_device, m_swapChain, m_commandPool),
          m_editor(*this)

    {
        // Camera
        m_camera.init(
            &m_window.input(),
            m_swapChain->extent().width / float(m_swapChain->extent().height)
        );

        // Global Descriptor Pool
        m_globalDescriptorPool = DescriptorPool::Builder(m_device)
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                                 .setMaxSets(1000)
                                 .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                                 .build();

        // Camera UBO
        m_cameraUboBuffer = std::make_shared<Buffer>(
            m_device,
            sizeof(CameraUBO),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        m_cameraUboBuffer->map();

        EventManager::subscribe<WindowResizeEvent>([this](WindowResizeEvent e)
        {
            m_dirtySwapChain = true;
            m_newWindowExtent = {static_cast<uint32_t>(e.newWidth), static_cast<uint32_t>(e.newHeight)};
        });

        EventManager::subscribe<ViewportResizeEvent>([this](ViewportResizeEvent e)
        {
            m_dirtyViewport = true;
            m_newViewportExtent = {static_cast<uint32_t>(e.newWidth), static_cast<uint32_t>(e.newHeight)};
        });
    }

    Engine::~Engine()
    {
        m_renderGraph.clear();
        m_assets.clearAll();
    }

    void Engine::registerRenderPass(Ref<IRenderPass> pass)
    {
        m_renderGraph.addPass(pass);
    }

    Ref<DescriptorSetLayout> Engine::getDescriptorSetLayout(const std::string& name) const
    {
        return m_descriptorSetLayouts.at(name);
    }

    void Engine::updateCameraUBO()
    {
        CameraUBO ubo{};
        ubo.view = m_camera.view();
        ubo.proj = m_camera.proj();
        ubo.position = m_camera.getPosition();

        m_cameraUboBuffer->writeToBuffer(&ubo, sizeof(ubo));
    }

    void Engine::drawFrame()
    {
        m_renderGraph.execute();
    }

    void Engine::onInit()
    {
        // "camera" layout (Set 0) for camera UBO
        // Matches: layout(set = 0, binding = 0) uniform CameraUBO
        m_descriptorSetLayouts["camera"] = vks::DescriptorSetLayout::Builder(m_device)
                                           .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                                           .build();

        // "material" layout (Set 1) for material UBO
        // Matches: layout(set = 1, binding = 0) uniform MaterialUBO
        m_descriptorSetLayouts["material"] = vks::DescriptorSetLayout::Builder(m_device)
                                             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                         VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT)
                                             .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                         VK_SHADER_STAGE_FRAGMENT_BIT)
                                             .build();

        // render target for ui pass
        auto objectPickingTarget = std::make_shared<RenderTarget>(
            device(),
            renderer().getSwapChain()->extent(),
            renderer().getSwapChain()->numImages(),
            VK_FORMAT_R32_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT
        );

        viewportTarget = std::make_shared<RenderTarget>(
            device(),
            renderer().getSwapChain()->extent(),
            renderer().getSwapChain()->numImages(),
            renderer().getSwapChain()->colorFormat(),
            renderer().getSwapChain()->depthFormat(),
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        );

        viewportRenderTargets.push_back(objectPickingTarget);
        viewportRenderTargets.push_back(viewportTarget);

        // Create passes
        auto geometryPass = std::make_shared<GeometryPass>(
            device(),
            viewportTarget
        );

        auto imguiPass = std::make_shared<ImGuiRenderPass>(
            device(),
            renderer().getSwapChain()
        );

        auto uiPass = std::make_shared<UIPass>(
            device(),
            objectPickingTarget
        );

        registerRenderPass(geometryPass);
        registerRenderPass(imguiPass);
        registerRenderPass(uiPass);

        // Create pipelines

        // Grid pipeline
        GraphicsPipelineDesc gridPipelineDesc{};
        gridPipelineDesc.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        gridPipelineDesc.isVertexInput = false;
        gridPipelineDesc.renderPass = geometryPass->handle();
        gridPipelineDesc.vertexShader = "assets/shaders/grid.vert.spv";
        gridPipelineDesc.fragmentShader = "assets/shaders/grid.frag.spv";
        gridPipelineDesc.alphaBlending = true;
        gridPipelineDesc.cull = VK_CULL_MODE_NONE;
        gridPipelineDesc.depthCompare = VK_COMPARE_OP_LESS;
        gridPipelineDesc.depthTest = true;
        gridPipelineDesc.depthWrite = true;
        gridPipelineDesc.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        gridPipelineDesc.viewportExtent = renderer().getSwapChain()->extent();
        gridPipelineDesc.dynamicStates.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);

        // Sphere pipeline
        GraphicsPipelineDesc spherePipelineDesc{};
        spherePipelineDesc.renderPass = geometryPass->handle();
        spherePipelineDesc.vertexShader = "assets/shaders/sphere.vert.spv";
        spherePipelineDesc.fragmentShader = "assets/shaders/sphere.frag.spv";
        spherePipelineDesc.alphaBlending = false;
        spherePipelineDesc.cull = VK_CULL_MODE_BACK_BIT;
        spherePipelineDesc.depthCompare = VK_COMPARE_OP_LESS;
        spherePipelineDesc.depthTest = true;
        spherePipelineDesc.depthWrite = true;
        spherePipelineDesc.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        spherePipelineDesc.viewportExtent = renderer().getSwapChain()->extent();

        // Sprite pipeline
        GraphicsPipelineDesc spritePipelineDesc{};
        spritePipelineDesc.renderPass = geometryPass->handle();
        spritePipelineDesc.vertexShader = "assets/shaders/sprite.vert.spv";
        spritePipelineDesc.fragmentShader = "assets/shaders/sprite.frag.spv";
        spritePipelineDesc.alphaBlending = true;
        spritePipelineDesc.cull = VK_CULL_MODE_NONE;
        spritePipelineDesc.depthCompare = VK_COMPARE_OP_LESS;
        spritePipelineDesc.depthTest = true;
        spritePipelineDesc.depthWrite = true;
        spritePipelineDesc.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        spritePipelineDesc.viewportExtent = renderer().getSwapChain()->extent();

        PipelineDesc gridPipelineDesc_{};
        gridPipelineDesc_.type = PipelineType::Graphics;
        gridPipelineDesc_.payload = gridPipelineDesc;
        gridPipelineDesc_.setLayouts = {
            m_descriptorSetLayouts["camera"]->getDescriptorSetLayout(),
            m_descriptorSetLayouts["material"]->getDescriptorSetLayout()
        };

        PipelineDesc spherePipelineDesc_{gridPipelineDesc_};
        spherePipelineDesc_.payload = spherePipelineDesc;
        spherePipelineDesc_.pushConstants = {
            VkPushConstantRange{
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(glm::mat4)
            }
        };

        PipelineDesc spritePipelineDesc_{gridPipelineDesc_};
        spritePipelineDesc_.payload = spritePipelineDesc;
        spritePipelineDesc_.pushConstants = {
            VkPushConstantRange{
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(glm::mat4)
            }
        };

        GraphicsPipelineDesc outlinePipelineDesc = spherePipelineDesc;
        outlinePipelineDesc.vertexShader = "assets/shaders/outline.vert.spv";
        outlinePipelineDesc.fragmentShader = "assets/shaders/outline.frag.spv";
        outlinePipelineDesc.cull = VK_CULL_MODE_BACK_BIT;
        outlinePipelineDesc.alphaBlending = true;
        // Cull front faces to create outline effect

        PipelineDesc outlinePipelineDesc_{spherePipelineDesc_};
        outlinePipelineDesc_.payload = outlinePipelineDesc;
        outlinePipelineDesc_.pushConstants = {
            VkPushConstantRange{
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = 96 // model matrix + outline width + outline color
            }
        };

        geometryPass->pipelines().createOrReplace("grid", gridPipelineDesc_);
        geometryPass->pipelines().createOrReplace("sphere", spherePipelineDesc_);
        geometryPass->pipelines().createOrReplace("sprite", spritePipelineDesc_);
        geometryPass->pipelines().createOrReplace("outline", outlinePipelineDesc_);

        GraphicsPipelineDesc uiPipelineDesc{};
        uiPipelineDesc.renderPass = uiPass->handle();
        uiPipelineDesc.vertexShader = "assets/shaders/objectPicking.vert.spv";
        uiPipelineDesc.fragmentShader = "assets/shaders/objectPicking.frag.spv";
        uiPipelineDesc.alphaBlending = false;
        uiPipelineDesc.cull = VK_CULL_MODE_NONE;
        uiPipelineDesc.depthCompare = VK_COMPARE_OP_LESS;
        uiPipelineDesc.depthTest = true;
        uiPipelineDesc.depthWrite = false; // Important for UI
        uiPipelineDesc.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        uiPipelineDesc.viewportExtent = renderer().getSwapChain()->extent();

        PipelineDesc uiPipelineDesc_{};
        uiPipelineDesc_.type = PipelineType::Graphics;
        uiPipelineDesc_.payload = uiPipelineDesc;
        uiPipelineDesc_.setLayouts = {
            m_descriptorSetLayouts["camera"]->getDescriptorSetLayout()
        };
        uiPipelineDesc_.pushConstants = {
            VkPushConstantRange{
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(glm::mat4) + sizeof(entt::entity)
            },
        };

        uiPass->pipelines().createOrReplace("ObjectPicker", uiPipelineDesc_);

        m_editor.onInit();
    }

    void Engine::run(Application& app)
    {
        // Initialize the engine context for global access
        EngineContext::set(this);
        onInit();

        // Let app setup engine
        app.onInit(*this);

        m_window.setDrawFrameFunc([this, &app](float dt)
        {
            handleRecreate();

            // App logic
            app.tick();

            updateCameraUBO();

            // UI
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            auto flags = ImGuiDockNodeFlags_PassthruCentralNode;
            ImGui::DockSpaceOverViewport(0, nullptr, flags);

            m_editor.onGui();

            app.onImGui();

            m_camera.update(dt, m_editor.isViewportInputAllowed());

            ImGui::Render();

            // Render
            drawFrame();
        });

        m_window.mainLoop();
        vkDeviceWaitIdle(m_device.logical());
    }

    void Engine::onImGui()
    {
    }

    void Engine::handleRecreate()
    {
        bool needRecreate = false;
        if (m_dirtySwapChain || m_dirtyViewport)
        {
            vkDeviceWaitIdle(m_device.logical());
            needRecreate = true;
        }

        if (needRecreate)
        {
            for (auto& target : viewportRenderTargets)
                target->resize(m_newViewportExtent);
            renderer().recreatePasses();
            m_dirtyViewport = false;
        }

        if (m_dirtySwapChain)
        {
            renderer().recreate();
            m_dirtySwapChain = false;
        }
    }
}
