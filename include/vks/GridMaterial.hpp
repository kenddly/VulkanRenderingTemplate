#pragma once

#include <vks/Material.hpp>

#define MAX_SPHERE_COUNT 16

namespace vks
{
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
    };


    class GridMaterial : public TypedMaterial<GridMaterialUBO>
    {
    public:
        using TypedMaterial::TypedMaterial;

        void update() override
        {
            uboData.time += 0.016;
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
            
            if (c) flush();

            ImGui::DragInt("Thickness", &thickness, 1, 1, 10);
        }

        void draw(VkCommandBuffer cmd, VkPipelineLayout layout, VkDescriptorSet& lastSet,
                  const vks::Model* model, const glm::mat4& transform, const vks::Camera& camera) override
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

            vkCmdDraw(cmd, 2, instanceCount, 0, 0);
        }

    private:
        int thickness = 1;
    };
}
