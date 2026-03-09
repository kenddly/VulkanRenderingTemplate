#pragma once

// ============================================================
//  PhysicsDebugRenderer.hpp
//
//  Implements JPH::DebugRenderer to collect collider wireframe
//  lines from Jolt, then draws them as an ImGui overlay on the
//  viewport using world-to-screen projection.
//
//  Usage:
//    1. Call init() once after PhysicsSystem::onInit()
//    2. Call render(scene, viewProj) each frame AFTER physics update
//    3. Toggle m_enabled from an ImGui checkbox
// ============================================================

// Jolt requires this before any other Jolt headers
#include <Jolt/Jolt.h>

// Enable the debug renderer interface in Jolt
#ifndef JPH_DEBUG_RENDERER
#define JPH_DEBUG_RENDERER
#endif

#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyManager.h>

#include <imgui.h>
#include <glm/glm.hpp>
#include <vector>

namespace vks
{
    class PhysicsDebugRenderer final : public JPH::DebugRenderer
    {
    public:
        // ---- Config ----------------------------------------
        bool m_enabled         = true;
        bool m_drawStatic      = false; // static bodies are usually noisy
        ImU32 m_dynamicColor   = IM_COL32(0, 255, 80,  200);
        ImU32 m_staticColor    = IM_COL32(100, 200, 0, 120);
        ImU32 m_sleepingColor  = IM_COL32(100, 100, 100, 150);

        // ---- Lifecycle -------------------------------------
        void init();

        /**
         * @brief Call every frame after PhysicsSystem::update().
         * @param joltSystem  The raw JPH::PhysicsSystem from getJoltSystem()
         * @param viewProj    Combined view-projection matrix from your camera
         * @param drawList    ImGui draw list — pass ImGui::GetBackgroundDrawList()
         *                    or the draw list of your viewport window
         * @param screenSize  Viewport size in pixels
         */
        void render(
            JPH::PhysicsSystem& joltSystem,
            const glm::mat4&    viewProj,
            ImDrawList*         drawList,
            glm::vec2           screenSize,
            glm::vec2           screenOffset = {0.f, 0.f});

        // ---- JPH::DebugRenderer interface ------------------
        // Jolt calls these while drawing shapes.
        // We collect into m_lines and flush in render().

        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;

        void DrawTriangle(JPH::RVec3Arg, JPH::RVec3Arg, JPH::RVec3Arg, JPH::ColorArg, ECastShadow) override;

        Batch CreateTriangleBatch(const Triangle*, int) override;
        Batch CreateTriangleBatch(const Vertex*, int, const JPH::uint32*, int) override;
        void DrawGeometry(JPH::RMat44Arg, const JPH::AABox&, float, JPH::ColorArg,
                          const GeometryRef&, ECullMode, ECastShadow, EDrawMode) override;

        void DrawText3D(JPH::RVec3Arg, const JPH::string_view&,
                        JPH::ColorArg, float) override {}

        void clear();
    private:
        struct Line
        {
            glm::vec3 a, b;
            ImU32     color;
        };

        std::vector<Line> m_lines;

        // Minimal BatchImpl — we don't use triangle batches
        class BatchImpl : public JPH::RefTargetVirtual
        {
        public:
            void AddRef()  override {}
            void Release() override { delete this; }
        };

        // Project a world-space point to screen pixels.
        // Returns false if the point is behind the camera.
        bool worldToScreen(
            const glm::vec3& world,
            const glm::mat4& viewProj,
            glm::vec2        screenSize,
            glm::vec2        screenOffset,
            ImVec2&          outScreen) const;
    };

} // namespace vks