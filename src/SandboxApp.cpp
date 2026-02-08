#include "SandboxApp.hpp"
#include "vks/Engine.hpp"

#include <vks/Model.hpp>
#include <vks/Material.hpp>
#include <vks/Render/GeometryPass.hpp>
#include <vks/ImGui/ImGuiRenderPass.hpp>

#include "vks/GridMaterial.hpp"
#include "vks/RenderObject.hpp"
#include "vks/SpriteMaterial.hpp"
#include "vks/Texture.hpp"

namespace vks
{
    void SandboxApp::onInit(Engine& engine)
    {
        loadAssets(engine);
        buildScene(engine);
    }

    void SandboxApp::loadAssets(Engine& engine)
    {
        auto cameraDescSetLayout = engine.getDescriptorSetLayout("camera");
        auto bufferInfo = engine.cameraBuffer()->descriptorInfo();
        DescriptorWriter(cameraDescSetLayout, engine.globalDescriptorPool())
            .writeBuffer(0, &bufferInfo)
            .build(engine.cameraDescriptorSet());

        auto& assets = engine.assets();

        Model sphere{};
        sphere.createSphere(1.0f, 32, 16);
        Model quad{};
        quad.createQuad();

        assets.add<Model>("sphere", std::move(sphere));
        assets.add<Model>("quad", std::move(quad));

        Ref<Texture> spriteTexture = std::make_shared<Texture>(
            engine.device(),
            "assets/textures/player.png"
        );
        Ref<Texture> spongeBobTexture = std::make_shared<Texture>(
            engine.device(),
            "assets/textures/Spongebobplush.png"
        );

        assets.add<Ref<Texture>>("sprite_texture", spriteTexture);
        assets.add<Ref<Texture>>("spongebob_texture", spongeBobTexture);

        // --- Red material ---
        auto redMaterial = std::make_shared<ColorMaterial>(
            engine.device(),
            "sphere",
            ColorMaterialUBO{glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}}
        );

        // --- Blue material ---
        auto blueMaterial = std::make_shared<ColorMaterial>(
            engine.device(),
            "sphere",
            ColorMaterialUBO{glm::vec4{0.0f, 122.0f / 255.0f, 1.0f, 1.0f}}
        );

        // --- Grid material ---
        auto gridMaterial = std::make_shared<GridMaterial>(
            engine.device(),
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

        SpriteMaterialUBO ubo{};
        ubo.tint = glm::vec4(1.0f);

        auto spriteMaterial = std::make_shared<SpriteMaterial>( engine.device(), spongeBobTexture, "sprite", ubo ); // Store in asset manager
        assets.add<Ref<Material>>("red_sphere", redMaterial);
        assets.add<Ref<Material>>("blue_sphere", blueMaterial);
        assets.add<Ref<Material>>("sprite", spriteMaterial);
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

        // GRID
        {
            auto& obj = scene.create("grid");

            obj.model = nullptr; // Procedural
            obj.material = assets
                           .get<Ref<Material>>("grid")
                           .get();

            obj.transform = glm::mat4(1.0f);
        }

        // SPRITE
        {
            auto& obj = scene.create("sprite");

            obj.model = &assets.get<Model>("quad"); // Reuse quad model
            obj.material = assets
                           .get<Ref<Material>>("sprite")
                           .get();

            obj.transform = glm::translate(
                glm::mat4(1.0f),
                glm::vec3{-2.0f, 0.0f, 0.0f}
            );
        }
    }

    void SandboxApp::onUpdate(float dt)
    {
    }

    void SandboxApp::onImGui()
    {
    }
}
