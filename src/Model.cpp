#include <vks/Model.hpp>

#include <vks/Application.hpp>
#include <vks/CommandBuffers.hpp>

#include "vks/EngineContext.hpp"
using namespace vks;

#include <vks/Model.hpp>

#include <vks/CommandBuffers.hpp>
#include <vks/Geometry.hpp>

using namespace vks;

// ============================================================
// PUBLIC API
// ============================================================

void Model::createSphere(
    float radius,
    uint32_t sectors,
    uint32_t stacks)
{
    std::vector<geometry::Vertex> vertices;
    std::vector<uint32_t> indices;

    geometry::createSphere(vertices, indices, radius, sectors, stacks);
    upload(vertices, indices);
}

void Model::createQuad()
{
    std::vector<geometry::Vertex> vertices;
    std::vector<uint32_t> indices;

    geometry::createQuad(vertices, indices);
    upload(vertices, indices);
}

// ============================================================
// INTERNAL SHARED UPLOAD
// ============================================================

void Model::bind(VkCommandBuffer cmd) const
{
    VkBuffer vb[] = { getVertexBuffer() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(cmd, 0, 1, vb, offsets);
    vkCmdBindIndexBuffer(cmd, getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, getIndexCount(), 1, 0, 0, 0);
}

void Model::upload(
    const std::vector<geometry::Vertex>& vertices,
    const std::vector<uint32_t>& indices)
{
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    m_indexCount  = static_cast<uint32_t>(indices.size());

    VkDeviceSize vertexSize = sizeof(vertices[0]) * m_vertexCount;
    VkDeviceSize indexSize  = sizeof(indices[0]) * m_indexCount;

    createBufferFromData(
        (void*)vertices.data(),
        vertexSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        m_vertexBuffer
    );

    createBufferFromData(
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
    void* data,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    std::unique_ptr<Buffer>& outBuffer)
{
    auto& ec = EngineContext::get();

    Buffer stagingBuffer{
        ec.device(),
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer(data);
    stagingBuffer.unmap();

    outBuffer = std::make_unique<Buffer>(
        ec.device(),
        size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    CommandBuffers::SingleTimeCommands(ec.device(), [&](VkCommandBuffer cmd) {
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

