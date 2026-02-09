#include <vks/SpriteMaterial.hpp>

#include "vks/EngineContext.hpp"

namespace vks
{
    SpriteMaterial::SpriteMaterial(
        std::shared_ptr<Texture> texture,
        const std::string& pipelineName,
        SpriteMaterialUBO initialData
    )
        : TypedMaterial(pipelineName, sizeof(SpriteMaterialUBO)),
          m_texture(texture)
    {
        buildDescriptorSet();
    }

    void SpriteMaterial::draw(
        VkCommandBuffer cmd,
        VkPipelineLayout layout,
        VkDescriptorSet& lastSet,
        const vks::Model* model,
        const glm::mat4& transform
    )
    {
        if (m_materialDescriptorSet != lastSet)
        {
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

        if (model)
        {
            model->bind(cmd);
        }
    }

    void SpriteMaterial::drawImguiEditor()
    {
        // 1. Draw Tint Color Picker
        bool c = false;
        c |= ImGui::ColorEdit4("Tint", &uboData.tint[0]);
        if (c) flush();

        ImGui::Separator();

        // 2. Texture Slot UI
        ImGui::Text("Sprite Texture:");

        ImTextureID iconId = EditorResourceManager::Instance().getIcon(m_texture->path);
        ImGui::Image(
            iconId,
            ImVec2(100, 100),
            ImVec2(0, 0),
            ImVec2(1, 1)
        );
        ImGui::Text("Image");

        // DRAG AND DROP TARGET
        if (ImGui::BeginDragDropTarget())
        {
            // Accept payload from AssetBrowserPanel
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const char* path = (const char*)payload->Data;
                std::filesystem::path filePath(path);

                // Filter extensions
                if (filePath.extension() == ".png" || filePath.extension() == ".jpg")
                {
                    // 4. Load Texture via Engine Asset Manager
                    auto& engine = EngineContext::get();

                    // Check if already loaded, or load new
                    std::string assetName = filePath.stem().string();

                    if (engine.assets().contains<std::shared_ptr<Texture>>(assetName))
                    {
                        auto tex = engine.assets().get<std::shared_ptr<Texture>>(assetName);
                        setTexture(tex);
                    }
                    else
                    {
                        // Load new texture
                        auto tex = std::make_shared<Texture>(engine.device(), filePath.string());
                        engine.assets().add<std::shared_ptr<Texture>>(assetName, tex);
                        setTexture(tex);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    Ref<Material> SpriteMaterial::clone() const
    {
        auto instance = std::make_shared<SpriteMaterial>(
            m_texture,
            this->m_pipelineName,
            this->uboData
        );
        instance->layer_priority = this->layer_priority;
        return instance;
    }


    void SpriteMaterial::setTexture(const Ref<Texture>& newTexture)
    {
        if (newTexture)
        {
            m_texture = newTexture;
            buildDescriptorSet(); // CRITICAL: Re-point descriptor to new image
        }
    }

    void SpriteMaterial::buildDescriptorSet()
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

        if (!writer.build(m_materialDescriptorSet))
        {
            throw std::runtime_error("Failed to build sprite material descriptor set");
        }
    }
}
