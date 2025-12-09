#include <vks/Material.hpp>

namespace vks {

    Material::Material(
        const vks::Device& device,
        const vks::GeometryPipeline& pipelineManager,
        Ref<vks::DescriptorPool> descriptorPool,
        const std::string& pipelineName,
        VkDeviceSize uboSize
    ) : m_pipelineName(pipelineName),
        m_materialDescriptorSet(VK_NULL_HANDLE)
    {
        // 1. Get the Descriptor Set Layout
        // Important: All pipelines used by Materials must share the same Set 1 Layout
        // (Binding 0 = Uniform Buffer)
        Ref<vks::DescriptorSetLayout> materialLayout =
            pipelineManager.getDescriptorSetLayout("material");

        // 2. Create the Uniform Buffer
        m_uboBuffer = std::make_unique<vks::Buffer>(
            device,
            uboSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        // Map the buffer persistently
        m_uboBuffer->map();

        // 3. Allocate and Write the Descriptor Set
        auto bufferInfo = m_uboBuffer->descriptorInfo();
    
        DescriptorWriter writer(materialLayout, descriptorPool);
        writer.writeBuffer(0, &bufferInfo); // Binds the UBO to Binding 0

        if (!writer.build(m_materialDescriptorSet)) {
            throw std::runtime_error("Failed to build descriptor set for material: " + pipelineName);
        }
    }

    void Material::writeToBuffer(const void* data, VkDeviceSize size) {
        // Relies on the buffer already being mapped in the constructor
        m_uboBuffer->writeToBuffer(const_cast<void*>(data), size);
    }

} // namespace vks