#pragma once

#include <imgui.h>
#include <scene/Components.hpp>
#include <editor/UI/ComponentsUI.hpp>
#include <materials/Material.hpp>

namespace vks
{
    template <>
    struct ComponentUI<Renderable>
    {
        static void draw(Scene& scene, entt::entity entity)
        {
            auto& registry = scene.getRegistry();
            auto& r = registry.get<Renderable>(entity);

            if (ImGui::TreeNode("Material"))
            {
                ImGui::Text("Material: %s", r.material->getPipelineName().c_str());
                r.material->drawImguiEditor();
                ImGui::TreePop();
            }
        }
    };
}
