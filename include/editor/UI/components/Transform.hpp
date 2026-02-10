#pragma once

#include <glm/glm.hpp>

#include <imgui.h>
#include <editor/UI/ComponentsUI.hpp>

namespace vks
{
    template <>
    struct ComponentUI<Transform>
    {
        static void draw(Scene& scene, entt::entity entity)
        {
            auto& registry = scene.getRegistry();
            auto& t = registry.get<Transform>(entity);

            bool dirty = false;

            dirty |= ImGui::DragFloat3("Position", &t.position.x, 0.1f);
            dirty |= ImGui::DragFloat3("Rotation", &t.rotation.x, 0.1f);
            dirty |= ImGui::DragFloat3("Scale", &t.scale.x, 0.1f, 0.01f, 100.0f);

            if (dirty)
            {
                t.updateTransform();
            }
        }
    };
}
