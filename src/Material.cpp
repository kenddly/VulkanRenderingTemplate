#include <vks/Material.hpp>

#include "vks/EngineContext.hpp"

namespace vks {

    Material::Material(
        const std::string& pipelineName,
        VkDeviceSize uboSize
    ) : m_pipelineName(pipelineName),
        m_materialDescriptorSet(VK_NULL_HANDLE)
    {
        auto& ec = EngineContext::get();
        auto descriptorPool = ec.globalDescriptorPool();
        auto materialLayout = ec.getDescriptorSetLayout("material");

        // 2. Create the Uniform Buffer
        m_uboBuffer = std::make_unique<vks::Buffer>(
            ec.device(),
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