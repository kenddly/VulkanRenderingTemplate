#include <app/SandboxApp.hpp>
#include <app/Engine.hpp>

#include <scene/Model.hpp>
#include <render/RenderObject.hpp>
#include <render/passes/GeometryPass.hpp>

#include <gfx/Texture.hpp>
#include <materials/Material.hpp>
#include <materials/ColorMaterial.hpp>
#include <materials/GridMaterial.hpp>
#include <materials/SpriteMaterial.hpp>

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

        Ref<Model> sphere = std::make_shared<Model>();
        sphere->createSphere(1.0f, 32, 16);
        Ref<Model> quad = std::make_shared<Model>();
        quad->createQuad();

        assets.add<Ref<Model>>("sphere", sphere);
        assets.add<Ref<Model>>("quad", quad);

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
            "sphere",
            ColorMaterialUBO{glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}}
        );

        // --- Blue material ---
        auto blueMaterial = std::make_shared<ColorMaterial>(
            "sphere",
            ColorMaterialUBO{glm::vec4{0.0f, 122.0f / 255.0f, 1.0f, 1.0f}}
        );

        // --- Grid material ---
        auto gridMaterial = std::make_shared<GridMaterial>(
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

        auto spriteMaterial = std::make_shared<SpriteMaterial>( spongeBobTexture, "sprite", ubo ); // Store in asset manager
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
            auto obj = scene.createEntity("red_sphere");
            auto transform = scene.getComponent<Transform>(obj);
            transform.transform = glm::mat4(1.0f);

            auto& renderable = scene.addComponent<Renderable>(obj);
            renderable.model = assets.get<Ref<Model>>("sphere");
            renderable.material = assets.get<Ref<Material>>("red_sphere");
        }

        // GRID
        {
            auto obj = scene.createEntity("grid");

            auto& renderable = scene.addComponent<Renderable>(obj);
            renderable.model = nullptr; // Procedural, no model
            renderable.material = assets.get<Ref<Material>>("grid");
        }

        // SPRITE
        {
            auto obj = scene.createEntity("sprite");
            auto transform = scene.getComponent<Transform>(obj);
            transform.transform = glm::translate(
                glm::mat4(1.0f),
                glm::vec3{-2.0f, 0.0f, 0.0f}
            );

            auto& renderable = scene.addComponent<Renderable>(obj);
            renderable.model = assets.get<Ref<Model>>("quad"); // Reuse quad model
            renderable.material = assets.get<Ref<Material>>("sprite");
        }
    }

    void SandboxApp::onUpdate(float dt)
    {
    }

    void SandboxApp::onImGui()
    {
    }
}
