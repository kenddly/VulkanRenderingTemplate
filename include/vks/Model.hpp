#ifndef MODEL_HPP
#define MODEL_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include <vks/Device.hpp>
#include <vks/CommandPool.hpp>
#include <vks/Geometry.hpp>

namespace vks
{
    class Model
    {
    public:
        Model(const Device& device, const CommandPool& commandPool, const std::vector<geometry::Vertex>& vertices,
              const std::vector<uint32_t>& indices);
        ~Model();

        void bind(VkCommandBuffer commandBuffer);

        uint32_t getIndexCount() const { return m_indexCount; }

        void createVertexBuffer(const std::vector<geometry::Vertex>& vertices);
        void createIndexBuffer(const std::vector<uint32_t>& indices);
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                          VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        const Device& m_device;
        const CommandPool& m_commandPool;

        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
        VkBuffer m_indexBuffer;
        VkDeviceMemory m_indexBufferMemory;

        uint32_t m_indexCount;
    };
} // namespace vks

#endif // MODEL_HPP