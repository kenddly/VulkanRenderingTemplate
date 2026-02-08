#include <vks/SpriteMaterial.hpp>

#include "vks/EngineContext.hpp"

namespace vks {

    SpriteMaterial::SpriteMaterial(
        const vks::Device& device,
        std::shared_ptr<Texture> texture,
        const std::string& pipelineName,
        SpriteMaterialUBO initialData
    )
        : TypedMaterial(device, pipelineName, sizeof(SpriteMaterialUBO)),
          m_texture(texture)
    {
        auto& ce = EngineContext::get();
        auto descriptorPool = ce.globalDescriptorPool();
        auto materialLayout = ce.getDescriptorSetLayout("material");

        DescriptorWriter writer(materialLayout, descriptorPool);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_texture->getImageView();
        imageInfo.sampler = m_texture->getSampler();

        writer.writeImage(1, &imageInfo);

        if (!writer.build(m_materialDescriptorSet)) {
            throw std::runtime_error("Failed to build sprite material descriptor set");
        }
    }

    void SpriteMaterial::draw(
        VkCommandBuffer cmd,
        VkPipelineLayout layout,
        VkDescriptorSet& lastSet,
        const vks::Model* model,
        const glm::mat4& transform
    ) {
        if (m_materialDescriptorSet != lastSet) {
            vkCmdBindDescriptorSets(
                cmd,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                layout,
                1, 1,
                &m_materialDescriptorSet,
                0, nullptr
            );
            lastSet = m_materialDescriptorSet;
        }

        vkCmdPushConstants(
            cmd,
            layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(glm::mat4),
            &transform
        );

        if (model) {
            model->bind(cmd);
        }
    }

    void SpriteMaterial::drawImguiEditor()
    {
        TypedMaterial<SpriteMaterialUBO>::drawImguiEditor();
        bool c = false;
        c |= ImGui::ColorEdit4("Tint", &uboData.tint[0]);

        if (c)
            flush();
    }
}
