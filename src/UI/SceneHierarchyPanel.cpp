#include <vks/UI/SceneHierarchyPanel.hpp>
#include <vks/Engine.hpp>
#include <imgui.h>

namespace vks
{
    void SceneHierarchyPanel::onGui()
    {
        if (!isOpen) return;

        ImGui::Begin(getTitle(), &isOpen);

        // Access scene via the Engine reference passed in constructor
        auto& renderObjects = m_engine.scene().objects();

        for (auto& pair : renderObjects)
        {
            // Unique ID generation for ImGui based on map key
            ImGui::PushID(pair.first.c_str());

            if (ImGui::TreeNode(pair.first.c_str()))
            {
                // We delegate the specific object drawing to the object itself
                // Or we could have an "InspectorPanel" handle this part
                pair.second.drawImguiEditor();
                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        ImGui::End();
    }
}
