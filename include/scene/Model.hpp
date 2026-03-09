#pragma once

#include <gfx/Buffer.hpp>
#include <scene/Geometry.hpp>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace vks
{
    class Model
    {
    public:
        Model() = default;
        ~Model() = default;

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        Model(Model&&) = default;
        Model& operator=(Model&&) = default;

        void createSphere(float radius, uint32_t sectors, uint32_t stacks);
        void createQuad();

        // --- Getters for the Render Loop ---
        VkBuffer getVertexBuffer() const { return m_vertexBuffer->getBuffer(); }
        VkBuffer getIndexBuffer()  const { return m_indexBuffer->getBuffer(); }
        uint32_t getIndexCount()   const { return m_indexCount; }
        void bind(VkCommandBuffer cmd) const;

        // --- CPU-side geometry (used by PhysicsSystem for mesh colliders) ---
        // These are kept in RAM after upload so the physics system can read them
        // without a GPU readback. The memory cost is negligible for typical meshes.
        const std::vector<geometry::Vertex>& getCPUVertices() const { return m_cpuVertices; }
        const std::vector<uint32_t>&         getCPUIndices()  const { return m_cpuIndices; }
        bool hasCPUGeometry() const { return !m_cpuVertices.empty(); }

    private:
        void upload(
            const std::vector<geometry::Vertex>& vertices,
            const std::vector<uint32_t>& indices);

        void createBufferFromData(
            void* data,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            std::unique_ptr<vks::Buffer>& outBuffer);

        std::unique_ptr<vks::Buffer> m_vertexBuffer;
        std::unique_ptr<vks::Buffer> m_indexBuffer;

        uint32_t m_vertexCount = 0;
        uint32_t m_indexCount  = 0;

        // CPU mirror — retained for physics/raycasting
        std::vector<geometry::Vertex> m_cpuVertices;
        std::vector<uint32_t>         m_cpuIndices;
    };
} // namespace vks