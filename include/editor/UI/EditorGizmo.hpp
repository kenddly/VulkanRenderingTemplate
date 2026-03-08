#pragma once
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/fwd.hpp>

namespace vks
{
    class Engine;
    struct Transform;

    class EditorGizmo
    {
    public:
        EditorGizmo(Engine& engine);
        void onGui(ImVec2 imageScreenPos, ImVec2 imageSize);
        bool isUsing() const { return m_usingGizmo; }

    private:
        Engine& m_engine;
        ImGuizmo::OPERATION m_operation = ImGuizmo::TRANSLATE;
        ImGuizmo::MODE m_mode = ImGuizmo::LOCAL;
        bool m_snap = false;
        bool m_usingGizmo = false;

        void handleShortcuts();
        static void writeBackTransform(Transform& transform, const glm::mat4& manipulated);
    };
}
