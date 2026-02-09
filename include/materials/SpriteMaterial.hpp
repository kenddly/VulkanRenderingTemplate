#pragma once
#include <gfx/Texture.hpp>
#include <materials/Material.hpp>

namespace vks {
    struct SpriteMaterialUBO
    {
        alignas(16) glm::vec4 tint{1.0f};
    };

    class SpriteMaterial : public TypedMaterial<SpriteMaterialUBO> {
    public:
        void buildDescriptorSet();
        SpriteMaterial(
            std::shared_ptr<Texture> texture,
            const std::string& pipelineName = "sprite",
            SpriteMaterialUBO initialData = {}
        );

        void draw(
            VkCommandBuffer cmd,
            VkPipelineLayout layout,
            VkDescriptorSet& lastSet,
            const vks::Model* model,
            const glm::mat4& transform
        ) override;

        void drawImguiEditor() override;
        Ref<Material> clone() const override;


        void setTexture(const std::shared_ptr<Texture>& newTexture);

    private:
        std::shared_ptr<Texture> m_texture;
    };
}
