#pragma once

#include <scene/Components.hpp>
#include <editor/UI/ComponentsUI.hpp>

namespace vks
{
    template<>
    struct ComponentUI<RigidBody>
    {
        static void draw(Scene& scene, entt::entity entity)
        {
            auto& registry = scene.getRegistry();
            auto& rb = registry.get<RigidBody>(entity);

            ImGui::DragFloat3("Velocity", &rb.velocity.x, 0.1f);
            ImGui::DragFloat3("Acceleration", &rb.acceleration.x, 0.1f);
            ImGui::DragFloat("Mass", &rb.mass, 0.1f, 0.01f, 100.0f);
        }
    };
}