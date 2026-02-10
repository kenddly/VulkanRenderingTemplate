#include <editor/UI/SceneHierarchyPanel.hpp>
#include <imgui.h>

#include <app/Engine.hpp>

#include <editor/UI/components/Name.hpp>
#include <editor/UI/components/RigidBody.hpp>
#include <editor/UI/components/Transform.hpp>
#include <editor/UI/components/Renderable.hpp>

namespace vks
{
    void SceneHierarchyPanel::onGui()
    {
        if (!isOpen) return;

        ImGui::Begin(getTitle(), &isOpen);

        auto& scene = m_engine.scene();
        auto renderObjects = scene.view<Transform, Name>();

        for (auto& object : renderObjects)
        {
            auto [transform, name] = renderObjects.get<Transform, Name>(object);
            auto cname = name.value.c_str();

            // Unique ID generation for ImGui based on map key
            ImGui::PushID(cname);

            if (ImGui::TreeNode(cname))
            {
                ComponentUI<Name>::draw(scene, object);
                ComponentUI<Transform>::draw(scene, object);

                if (scene.hasComponent<Renderable>(object))
                    ComponentUI<Renderable>::draw(scene, object);
                if (scene.hasComponent<RigidBody>(object))
                    ComponentUI<RigidBody>::draw(scene, object);

                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        ImGui::End();
    }
}
