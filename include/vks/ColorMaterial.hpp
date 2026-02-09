#pragma once
#include <vks/Material.hpp>

namespace vks
{
    // COLOR (Standard Mesh Rendering)
    struct ColorMaterialUBO
    {
        alignas(16) glm::vec4 color;
    };

    class ColorMaterial : public TypedMaterial<ColorMaterialUBO>
    {
    public:
        using TypedMaterial::TypedMaterial;

        void drawImguiEditor() override;

        void draw(VkCommandBuffer cmd, VkPipelineLayout layout, VkDescriptorSet& lastSet,
                  const Model* model, const glm::mat4& transform) override;

        std::shared_ptr<Material> clone() const override;
    };
}
