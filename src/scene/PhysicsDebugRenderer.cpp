// ============================================================
//  PhysicsDebugRenderer.cpp
// ============================================================

#include <scene/PhysicsDebugRenderer.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vks
{
    // ============================================================
    //  TriangleBatchImpl
    //  JPH_OVERRIDE_NEW_DELETE routes new/delete through Jolt's
    //  allocator — required for any RefTarget subclass, otherwise
    //  Jolt calls JPH::Free() on a non-Jolt pointer on shutdown.
    // ============================================================
    struct TriangleBatchImpl : public JPH::RefTargetVirtual
    {
        JPH_OVERRIDE_NEW_DELETE

        virtual void AddRef() override { ++mRefCount; }
        virtual void Release() override { if (--mRefCount == 0) delete this; }

        struct Tri
        {
            glm::vec3 v0, v1, v2;
        };

        std::vector<Tri> triangles;

    private:
        std::atomic<uint32_t> mRefCount = 0;
    };


    void PhysicsDebugRenderer::init()
    {
        JPH::DebugRenderer::Initialize();
    }

    void PhysicsDebugRenderer::clear()
    {
        m_lines.clear();
    }

    void PhysicsDebugRenderer::render(
        JPH::PhysicsSystem& joltSystem,
        const glm::mat4& viewProj,
        ImDrawList* drawList,
        glm::vec2 screenSize,
        glm::vec2 screenOffset)
    {
        if (!m_enabled || !drawList) return;

        m_lines.clear();

        JPH::BodyManager::DrawSettings drawSettings;
        drawSettings.mDrawShape = true;
        drawSettings.mDrawShapeWireframe = true;
        drawSettings.mDrawBoundingBox = false;
        drawSettings.mDrawCenterOfMassTransform = false;

        joltSystem.DrawBodies(drawSettings, this);

        for (const auto& line : m_lines)
        {
            ImVec2 screenA, screenB;
            if (!worldToScreen(line.a, viewProj, screenSize, screenOffset, screenA)) continue;
            if (!worldToScreen(line.b, viewProj, screenSize, screenOffset, screenB)) continue;
            drawList->AddLine(screenA, screenB, line.color, 1.5f);
        }

        m_lines.clear();
    }

    JPH::DebugRenderer::Batch PhysicsDebugRenderer::CreateTriangleBatch(
        const Triangle* inTriangles, int inTriangleCount)
    {
        auto* batch = new TriangleBatchImpl();
        batch->triangles.reserve(inTriangleCount);
        for (int i = 0; i < inTriangleCount; i++)
        {
            const auto& t = inTriangles[i];
            batch->triangles.push_back({
                {t.mV[0].mPosition.x, t.mV[0].mPosition.y, t.mV[0].mPosition.z},
                {t.mV[1].mPosition.x, t.mV[1].mPosition.y, t.mV[1].mPosition.z},
                {t.mV[2].mPosition.x, t.mV[2].mPosition.y, t.mV[2].mPosition.z}
            });
        }
        return batch;
    }

    JPH::DebugRenderer::Batch PhysicsDebugRenderer::CreateTriangleBatch(
        const Vertex* inVertices, int inVertexCount,
        const JPH::uint32* inIndices, int inIndexCount)
    {
        auto* batch = new TriangleBatchImpl();
        batch->triangles.reserve(inIndexCount / 3);
        for (int i = 0; i + 2 < inIndexCount; i += 3)
        {
            const auto& v0 = inVertices[inIndices[i]];
            const auto& v1 = inVertices[inIndices[i + 1]];
            const auto& v2 = inVertices[inIndices[i + 2]];
            batch->triangles.push_back({
                {v0.mPosition.x, v0.mPosition.y, v0.mPosition.z},
                {v1.mPosition.x, v1.mPosition.y, v1.mPosition.z},
                {v2.mPosition.x, v2.mPosition.y, v2.mPosition.z}
            });
        }
        return batch;
    }

    void PhysicsDebugRenderer::DrawGeometry(
        JPH::RMat44Arg inModelMatrix,
        const JPH::AABox& inWorldSpaceBounds,
        float inLODScaleSq,
        JPH::ColorArg inModelColor,
        const GeometryRef& inGeometry,
        ECullMode inCullMode,
        ECastShadow inCastShadow,
        EDrawMode inDrawMode)
    {
        if (inGeometry->mLODs.empty()) return;

        const auto* batch = static_cast<const TriangleBatchImpl*>(
            inGeometry->mLODs[0].mTriangleBatch.GetPtr());

        if (!batch) return;

        ImU32 col = IM_COL32(
            inModelColor.r, inModelColor.g,
            inModelColor.b, inModelColor.a);

        for (const auto& tri : batch->triangles)
        {
            auto transform = [&](const glm::vec3& v) -> glm::vec3
            {
                JPH::Vec3 out = inModelMatrix * JPH::Vec3(v.x, v.y, v.z);
                return {out.GetX(), out.GetY(), out.GetZ()}; // flip Y
            };

            glm::vec3 a = transform(tri.v0);
            glm::vec3 b = transform(tri.v1);
            glm::vec3 c = transform(tri.v2);

            m_lines.push_back({a, b, col});
            m_lines.push_back({b, c, col});
            m_lines.push_back({c, a, col});
        }
    }

    void PhysicsDebugRenderer::DrawTriangle(
        JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3,
        JPH::ColorArg inColor, ECastShadow inCastShadow)
    {
        ImU32 col = IM_COL32(inColor.r, inColor.g, inColor.b, inColor.a);
        glm::vec3 a(inV1.GetX(), inV1.GetY(), inV1.GetZ());
        glm::vec3 b(inV2.GetX(), inV2.GetY(), inV2.GetZ());
        glm::vec3 c(inV3.GetX(), inV3.GetY(), inV3.GetZ());
        m_lines.push_back({a, b, col});
        m_lines.push_back({b, c, col});
        m_lines.push_back({c, a, col});
    }

    void PhysicsDebugRenderer::DrawLine(
        JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
    {
        ImU32 col;
        if (inColor == JPH::Color::sGreen) col = m_dynamicColor;
        else if (inColor == JPH::Color::sGrey) col = m_sleepingColor;
        else if (inColor == JPH::Color::sYellow ||
            inColor == JPH::Color::sOrange)
            col = m_staticColor;
        else col = IM_COL32(inColor.r, inColor.g, inColor.b, inColor.a);

        m_lines.push_back({
            glm::vec3(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()),
            glm::vec3(inTo.GetX(), inTo.GetY(), inTo.GetZ()),
            col
        });
    }

    bool PhysicsDebugRenderer::worldToScreen(
        const glm::vec3& world, const glm::mat4& viewProj,
        glm::vec2 screenSize, glm::vec2 screenOffset, ImVec2& outScreen) const
    {
        glm::vec4 clip = viewProj * glm::vec4(world, 1.0f);
        if (clip.w <= 0.0f) return false;

        glm::vec3 ndc = glm::vec3(clip) / clip.w;
        if (ndc.x < -1.1f || ndc.x > 1.1f ||
            ndc.y < -1.1f || ndc.y > 1.1f)
            return false;

        outScreen.x = screenOffset.x + (ndc.x * 0.5f + 0.5f) * screenSize.x;
        outScreen.y = screenOffset.y + (1.0f - (ndc.y * 0.5f + 0.5f)) * screenSize.y;
        return true;
    }
} // namespace vks
