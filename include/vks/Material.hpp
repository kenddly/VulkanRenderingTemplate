#pragma once

#include <vks/GraphicsPipeline.hpp>
#include <vks/Descriptors.hpp>
#include <vks/Buffer.hpp>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <imgui.h>

#include <string>
#include <memory>
#include <stdexcept>

#include "Camera.hpp"
#include "Model.hpp"

namespace vks
{
// Forward declare Camera to avoid circular includes
class Camera; 

// =========================================================================
// 1. BASE MATERIAL CLASS
// =========================================================================
class Material {
public:
    virtual ~Material() = default;

    // Delete Copy (Materials own Vulkan Resources)
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    // Allow Move
    Material(Material&&) = default;
    Material& operator=(Material&&) = default;
    

    virtual void drawImguiEditor()
    {
        ImGui::DragInt("Layer Priority", &layer_priority, 1, -100, 100);
    }

    /**
     * @brief Records the specific draw commands for this material.
     * @param cmd The recording command buffer.
     * @param layout The current pipeline layout.
     * @param lastSet Reference to the last bound descriptor set (for optimization).
     * @param model Pointer to the model (can be nullptr for procedural).
     * @param transform The object's transform matrix.
     * @param camera The scene camera (needed for grid calculations).
     */
    virtual void draw(
        VkCommandBuffer cmd, 
        VkPipelineLayout layout, 
        VkDescriptorSet& lastSet,
        const vks::Model* model, 
        const glm::mat4& transform,
        const vks::Camera& camera
    ) = 0;

    /**
     * @brief Type-safe helper to cast base material to derived type.
     * Usage: if (auto* colorMat = mat->getAs<ColorMaterial>()) { ... }
     */
    template <typename T>
    T* getAs()
    {
        return dynamic_cast<T*>(this);
    }

    const std::string& getPipelineName() const { return m_pipelineName; }
    VkDescriptorSet getDescriptorSet() const { return m_materialDescriptorSet; }

    int layer_priority = 0;

protected:
    // Protected Constructor: Only derived classes can instantiate
    Material(
        const vks::Device& device,
        const vks::GraphicsPipeline& pipelineManager,
        vks::Ref<vks::DescriptorPool> descriptorPool,
        const std::string& pipelineName,
        VkDeviceSize uboSize
    );

    // Helper to upload data to the GPU
    void writeToBuffer(const void* data, VkDeviceSize size);

    std::string m_pipelineName;
    VkDescriptorSet m_materialDescriptorSet;

    // The Material owns this buffer
    std::unique_ptr<vks::Buffer> m_uboBuffer;
};

// =========================================================================
// 2. TEMPLATE WRAPPER
// =========================================================================
template <typename UBOStruct>
class TypedMaterial : public Material
{
public:
    UBOStruct uboData;
    using Material::Material; // Use parent constructors

    TypedMaterial(
        const vks::Device& device,
        const vks::GraphicsPipeline& pipelineManager,
        vks::Ref<vks::DescriptorPool> descriptorPool,
        const std::string& pipelineName,
        UBOStruct initialData
    ) : Material(device, pipelineManager, descriptorPool, pipelineName, sizeof(UBOStruct))
    {
        uboData = initialData;
        flush();
    }

    void flush()
    {
        writeToBuffer(&uboData, sizeof(UBOStruct));
    }
};

// =========================================================================
// 3. CONCRETE MATERIAL: COLOR (Standard Mesh Rendering)
// =========================================================================
struct ColorMaterialUBO
{
    alignas(16) glm::vec4 color;
};

class ColorMaterial : public TypedMaterial<ColorMaterialUBO>
{
public:
    using TypedMaterial::TypedMaterial;

    void drawImguiEditor() override
    {
        if (ImGui::ColorEdit4("Base Color", &uboData.color[0])) flush();
        Material::drawImguiEditor();
    }

    void draw(VkCommandBuffer cmd, VkPipelineLayout layout, VkDescriptorSet& lastSet,
              const vks::Model* model, const glm::mat4& transform, const vks::Camera& camera) override
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

            // 3. Bind Geometry & Draw
            VkBuffer vertexBuffers[] = {model->getVertexBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(cmd, model->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(cmd, model->getIndexCount(), 1, 0, 0, 0);
        }
    }
};

// =========================================================================
// 4. CONCRETE MATERIAL: GRID (Procedural Rendering)
// =========================================================================
struct GridMaterialUBO
{
    alignas(16) glm::vec4 color;
    alignas(4) float spacing;
    alignas(4) float dimension; // Radius of grid
    alignas(4) float thickness; // Radius of grid
};

class GridMaterial : public TypedMaterial<GridMaterialUBO>
{
public:
    using TypedMaterial::TypedMaterial;

    void drawImguiEditor() override
    {
        bool c = false;
        c |= ImGui::ColorEdit4("Color", &uboData.color[0]);
        c |= ImGui::DragFloat("Spacing", &uboData.spacing, 0.1f);
        c |= ImGui::DragFloat("Dimension", &uboData.dimension, 1.0f);
        c |= ImGui::DragFloat("Thickness", &uboData.thickness, 1.0f);
        if (c) flush();

        Material::drawImguiEditor();
    }

    void draw(VkCommandBuffer cmd, VkPipelineLayout layout, VkDescriptorSet& lastSet,
              const vks::Model* model, const glm::mat4& transform, const vks::Camera& camera) override
    {
        // Grid usually doesn't use the Descriptor Set 1 (Material UBO) in the shader
        // but we bind it anyway to satisfy layout compatibility if needed.
        if (m_materialDescriptorSet != lastSet)
        {
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    layout, 1, 1, &m_materialDescriptorSet, 0, nullptr);
            lastSet = m_materialDescriptorSet;
        }

        // 1. Prepare Push Constants specific to the Grid Shader
        struct GridPushData
        {
            glm::vec4 camPos; // Used for Camera Pos or Transform
            glm::vec4 color;
            glm::vec2 settings; // spacing, dimension, padding
        } push{};

        push.camPos = glm::vec4(camera.getPosition(), 0);
        push.color = uboData.color;
        push.settings = glm::vec2(uboData.spacing, uboData.dimension);

        vkCmdSetLineWidth(cmd, uboData.thickness);

        vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0, sizeof(GridPushData), &push);

        // 2. Procedural Draw Call (No Vertex Buffers)
        int S = 2 * (int)uboData.dimension + 1;
        int instanceCount = 3 * S * S; // 3 axes

        vkCmdDraw(cmd, 2, instanceCount, 0, 0);
    }
};
}
