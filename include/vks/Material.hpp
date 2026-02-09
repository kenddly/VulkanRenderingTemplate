#pragma once

#include <vks/Buffer.hpp>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <imgui.h>

#include <string>
#include <memory>

#include "Camera.hpp"
#include "Model.hpp"

namespace vks
{
    // Forward declare Camera to avoid circular includes
    class Camera;

    // BASE MATERIAL CLASS
    class Material
    {
    public:
        virtual ~Material() = default;

        // Delete Copy (Materials own Vulkan Resources)
        Material(const Material&) = delete;
        Material& operator=(const Material&) = delete;

        // Allow Move
        Material(Material&&) = default;
        Material& operator=(Material&&) = default;

        virtual void update()
        {
        }

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
            const Model* model,
            const glm::mat4& transform
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

        virtual std::shared_ptr<Material> clone() const = 0;

        const std::string& getPipelineName() const { return m_pipelineName; }
        VkDescriptorSet getDescriptorSet() const { return m_materialDescriptorSet; }

        int layer_priority = 0;

    protected:
        // Protected Constructor: Only derived classes can instantiate
        Material(
            const std::string& pipelineName,
            VkDeviceSize uboSize
        );

        // Helper to upload data to the GPU
        void writeToBuffer(const void* data, VkDeviceSize size);

        std::string m_pipelineName;
        VkDescriptorSet m_materialDescriptorSet;

        // The Material owns this buffer
        std::unique_ptr<Buffer> m_uboBuffer;
    };

    // TEMPLATE WRAPPER
    template <typename UBOStruct>
    class TypedMaterial : public Material
    {
    public:
        UBOStruct uboData;
        using Material::Material; // Use parent constructors

        TypedMaterial(
            const std::string& pipelineName,
            UBOStruct initialData
        ) : Material(pipelineName, sizeof(UBOStruct))
        {
            uboData = initialData;
            flush();
        }

        std::shared_ptr<Material> clone() const override
        {
            // Create a new object of the same Derived type
            // This triggers the constructor which allocates a NEW UBO and Descriptor Set
            auto instance = std::make_shared<TypedMaterial>(
                this->m_pipelineName,
                this->uboData
            );

            instance->layer_priority = this->layer_priority;
            return instance;
        }

        void flush()
        {
            uint32_t size = sizeof(UBOStruct);
            writeToBuffer(&uboData, size);
        }

        void draw(VkCommandBuffer cmd,
                  VkPipelineLayout layout,
                  VkDescriptorSet& lastSet,
                  const Model* model,
                  const glm::mat4& transform) override
        {}
    };

}
