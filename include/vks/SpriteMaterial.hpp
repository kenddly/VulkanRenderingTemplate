#pragma once
#include <vks/Material.hpp>
#include <vks/Texture.hpp>

namespace vks {
    struct SpriteMaterialUBO
    {
        alignas(16) glm::vec4 tint{1.0f};
    };

    class SpriteMaterial : public TypedMaterial<SpriteMaterialUBO> {
    public:
        SpriteMaterial(
            const vks::Device& device,
            const vks::GeometryPipeline& pipelineManager,
            Ref<vks::DescriptorPool> descriptorPool,
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

        void bind() override {}

    private:
        std::shared_ptr<Texture> m_texture;
    };

}
