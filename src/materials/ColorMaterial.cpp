#include <../include/materials/ColorMaterial.hpp>

void vks::ColorMaterial::drawImguiEditor()
{
    Material::drawImguiEditor();
    if (ImGui::ColorEdit4("Base Color", &uboData.color[0])) flush();
}

void vks::ColorMaterial::draw(VkCommandBuffer cmd, VkPipelineLayout layout, VkDescriptorSet& lastSet,
    const Model* model, const glm::mat4& transform)
{
    // 1. Bind Descriptor Set (Optimized)
    if (m_materialDescriptorSet != lastSet)
    {
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                layout, 1, 1, &m_materialDescriptorSet, 0, nullptr);
        lastSet = m_materialDescriptorSet;
    }

    if (model)
    {
        // 2. Push Constants (Model Matrix + Color)
        struct PushConstant
        {
            glm::mat4 model;
        } push{};
        push.model = transform;

        vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT,
                           0, sizeof(PushConstant), &push);

        model->bind(cmd);
    }
}

std::shared_ptr<vks::Material> vks::ColorMaterial::clone() const
{
    auto instance = std::make_shared<ColorMaterial>(
        this->m_pipelineName,
        this->uboData
    );
    instance->layer_priority = this->layer_priority;
    return instance;
}
