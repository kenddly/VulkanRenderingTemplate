#pragma once

#include <core/Time.hpp>
#include <app/EngineContext.hpp>
#include <materials/Material.hpp>

#define MAX_SPHERE_COUNT 16

namespace vks
{
    struct Sphere
    {
        alignas(16) glm::vec3 center;
        float radius;
        float mass;
    };

    constexpr int MAX_SPHERES = 32;

    struct GridMaterialUBO
    {
        alignas(16) glm::vec4 color;

        float spacing;
        int dimension;

        float glowStrength;
        float glowPower;

        float nearFade;
        float farFade;

        float time;

        Sphere spheres[MAX_SPHERES];
        int sphereCount;
        float softening;
        float curvatureK;

        int lineVertexCount;
        int integratorSteps;
    };


    class GridMaterial : public TypedMaterial<GridMaterialUBO>
    {
    public:
        using TypedMaterial::TypedMaterial;

        void update() override
        {
            auto renderObjects = EngineContext::get().scene().view<Renderable, Transform, RigidBody>();
            uboData.sphereCount = 0;
            for (auto obj : renderObjects)
            {
                auto [renderable, transform, rigidBody] = renderObjects.get<Renderable, Transform, RigidBody>(obj);

                if (renderable.material.get() == this)
                    continue;

                Sphere sphere{};
                sphere.center = transform.position;
                sphere.center.y *= -1.0f; // Invert Y for our coordinate system
                sphere.radius = transform.scale.x * 0.5f; // Assuming uniform scale
                sphere.mass = rigidBody.mass;

                if (uboData.sphereCount < MAX_SPHERES)
                {
                    uboData.spheres[uboData.sphereCount] = sphere;
                    uboData.sphereCount++;
                }
            }

            uboData.time = Time::getTotalTime();

            flush();
        }

        void drawImguiEditor() override
        {
            Material::drawImguiEditor();

            bool c = false;

            c |= ImGui::ColorEdit4("Color", &uboData.color[0]);
            c |= ImGui::DragFloat("Spacing", &uboData.spacing, 0.1f);
            c |= ImGui::DragInt("Dimension", &uboData.dimension, 1);
            c |= ImGui::DragFloat("Glow Strength", &uboData.glowStrength, 0.01f, 0.0f, 1.0f);
            c |= ImGui::DragFloat("Glow Power", &uboData.glowPower, 0.01f, 0.0f, 1.0f);
            c |= ImGui::DragFloat("Near Fade", &uboData.nearFade, 0.01f, 0.0f, 100.0f);
            c |= ImGui::DragFloat("Far Fade", &uboData.farFade, 0.01f, 0.0f, 500.0f);
            c |= ImGui::DragFloat("Softening", &uboData.softening, 0.01f, 0.0f, 500.0f);
            c |= ImGui::DragFloat("CurvatureK", &uboData.curvatureK, 0.01f, 0.0f, 500.0f);
            c |= ImGui::DragInt("Integrator Steps", &uboData.integratorSteps, 1, 16, 512);
            c |= ImGui::DragInt("SamplesPerLine", &uboData.lineVertexCount, 1, 16, 512);

            if (c) flush();

            ImGui::DragInt("Thickness", &thickness, 1, 1, 10);
        }

        void draw(VkCommandBuffer cmd, VkPipelineLayout layout, VkDescriptorSet lastSet,
                  const vks::Model* model, const glm::mat4& transform) override
        {
            if (m_materialDescriptorSet != lastSet)
            {
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        layout, 1, 1, &m_materialDescriptorSet, 0, nullptr);
                lastSet = m_materialDescriptorSet;
            }

            vkCmdSetLineWidth(cmd, thickness);

            int S = 2 * uboData.dimension + 1;
            int instanceCount = 3 * S * S;

            vkCmdDraw(cmd, samplesPerLine, instanceCount, 0, 0);
        }

    private:
        int thickness = 4;
        int integratorSteps = 64;
        int samplesPerLine = 128;
    };
}
