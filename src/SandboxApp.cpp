#include "SandboxApp.hpp"
#include "vks/Engine.hpp"

#include <vks/Model.hpp>
#include <vks/Material.hpp>
#include <vks/Render/GeometryPass.hpp>
#include <vks/ImGui/ImGuiRenderPass.hpp>

#include "Log.hpp"
#include "vks/GridMaterial.hpp"
#include "vks/RenderObject.hpp"

namespace vks
{
    void SandboxApp::onInit(Engine& engine)
    {
        loadAssets(engine);
        buildScene(engine);
    }

    void SandboxApp::loadAssets(Engine& engine)
    {
        auto geometryPass = std::make_shared<GeometryPass>(
            engine.device(),
            engine.renderer().getSwapChain()
        );

        auto ImguiRenderPass = std::make_shared<ImGuiRenderPass>(
            engine.device(),
            engine.renderer().getSwapChain()
        );


        // Register render passes
        engine.registerRenderPass(geometryPass);
        engine.registerRenderPass(ImguiRenderPass);

        auto cameraDescSetLayout = geometryPass->getPipeline()->getDescriptorSetLayout("camera");
        auto bufferInfo = engine.cameraBuffer()->descriptorInfo();
        vks::DescriptorWriter(cameraDescSetLayout, engine.globalDescriptorPool())
            .writeBuffer(0, &bufferInfo)
            .build(engine.cameraDescriptorSet());

        auto& assets = engine.assets();

        Model sphere{};
        sphere.createSphere(
            engine.device(),
            engine.commandPool().handle(),
            1.0f, 32, 16
        );
        assets.add<Model>("sphere", std::move(sphere));

        auto& pipeline = *geometryPass->getPipeline();

        // --- Red material ---
        auto redMaterial = std::make_shared<ColorMaterial>(
            engine.device(),
            pipeline,
            engine.globalDescriptorPool(),
            "sphere",
            ColorMaterialUBO{glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}}
        );
        redMaterial->layer_priority = 0;

        // --- Blue material ---
        auto blueMaterial = std::make_shared<ColorMaterial>(
            engine.device(),
            pipeline,
            engine.globalDescriptorPool(),
            "sphere",
            ColorMaterialUBO{glm::vec4{0.0f, 122.0f / 255.0f, 1.0f, 1.0f}}
        );
        blueMaterial->layer_priority = 0;

        // --- Grid material ---
        auto gridMaterial = std::make_shared<GridMaterial>(
            engine.device(),
            pipeline,
            engine.globalDescriptorPool(),
            "grid",
            GridMaterialUBO{
                glm::vec4{0.0f, 0.67f, 0.78f, 1.0f},
                5.0f,
                20,
                1.0f,
                0.75f,
                1.0f,
                100.0f,
                0.0f
            }
        );
        gridMaterial->layer_priority = -1;

        // Store in asset manager
        assets.add<Ref<Material>>("red_sphere", redMaterial);
        assets.add<Ref<Material>>("blue_sphere", blueMaterial);
        assets.add<Ref<Material>>("grid", gridMaterial);
    }

    void SandboxApp::buildScene(Engine& engine)
    {
        auto& scene = engine.scene();
        auto& assets = engine.assets();

        // RED SPHERE
        {
            auto& obj = scene.create("red_sphere");

            obj.model = &assets.get<Model>("sphere");
            obj.material = assets
                           .get<Ref<Material>>("red_sphere")
                           .get();

            obj.transform = glm::translate(
                glm::mat4(1.0f),
                glm::vec3{0.0f, 0.0f, 0.0f}
            );
        }

        // BLUE SPHERE
        {
            auto& obj = scene.create("blue_sphere");

            obj.model = &assets.get<Model>("sphere");
            obj.material = assets
                           .get<Ref<Material>>("blue_sphere")
                           .get();

            obj.transform = glm::translate(
                glm::mat4(1.0f),
                glm::vec3{2.0f, 0.0f, 0.0f}
            );
        }

        // -------------------------------------------------
        // GRID
        // -------------------------------------------------
        {
            auto& obj = scene.create("grid");

            obj.model = nullptr; // Procedural
            obj.material = assets
                           .get<Ref<Material>>("grid")
                           .get();

            obj.transform = glm::mat4(1.0f);
        }
    }

    void SandboxApp::onUpdate(float dt)
    {
    }

    void SandboxApp::onImGui()
    {
        ImGui::Begin("Sandbox");
        auto& renderObjects = EngineContext::get().scene().objects();

        for (auto& pair : renderObjects)
        {
            auto& obj = pair.second;

            if (ImGui::TreeNode(pair.first.c_str()))
            {
                obj.drawImguiEditor();
                ImGui::TreePop();
            }
        }

        ImGui::End();
    }
}
