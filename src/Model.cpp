#include <vks/Model.hpp>

#include <vks/Application.hpp>
#include <vks/CommandBuffers.hpp>
using namespace vks;

#include <vks/Model.hpp>

#include <vks/CommandBuffers.hpp>
#include <vks/Geometry.hpp>

using namespace vks;

// ============================================================
// PUBLIC API
// ============================================================

void Model::createSphere(
    const Device& device,
    VkCommandPool commandPool,
    float radius,
    uint32_t sectors,
    uint32_t stacks)
{
    std::vector<geometry::Vertex> vertices;
    std::vector<uint32_t> indices;

    geometry::createSphere(vertices, indices, radius, sectors, stacks);
    upload(device, commandPool, vertices, indices);
}

void Model::createQuad(
    const Device& device,
    VkCommandPool commandPool)
{
    std::vector<geometry::Vertex> vertices;
    std::vector<uint32_t> indices;

    geometry::createQuad(vertices, indices);
    upload(device, commandPool, vertices, indices);
}

// ============================================================
// INTERNAL SHARED UPLOAD
// ============================================================

void Model::upload(
    const Device& device,
    VkCommandPool commandPool,
    const std::vector<geometry::Vertex>& vertices,
    const std::vector<uint32_t>& indices)
{
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    m_indexCount  = static_cast<uint32_t>(indices.size());

    VkDeviceSize vertexSize = sizeof(vertices[0]) * m_vertexCount;
    VkDeviceSize indexSize  = sizeof(indices[0]) * m_indexCount;

    createBufferFromData(
        device,
        commandPool,
        (void*)vertices.data(),
        vertexSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        m_vertexBuffer
    );

    createBufferFromData(
        device,
        commandPool,
        (void*)indices.data(),
        indexSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        m_indexBuffer
    );
}

// ============================================================
// GPU BUFFER HELPER
// ============================================================

void Model::createBufferFromData(
    const Device& device,
    VkCommandPool commandPool,
    void* data,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    std::unique_ptr<Buffer>& outBuffer)
{
    Buffer stagingBuffer{
        device,
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer(data);
    stagingBuffer.unmap();

    outBuffer = std::make_unique<Buffer>(
        device,
        size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    CommandBuffers::SingleTimeCommands(device, [&](VkCommandBuffer cmd) {
        VkBufferCopy copy{};
        copy.size = size;
        vkCmdCopyBuffer(cmd,
            stagingBuffer.getBuffer(),
            outBuffer->getBuffer(),
            1,
            &copy
        );
    });
}

