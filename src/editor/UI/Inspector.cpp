#include <editor/UI/Inspector.hpp>

namespace vks
{
    InspectorPanel::InspectorPanel(Engine& engine) : IEditorPanel(engine), m_engine(engine), m_editor(engine.editor())
    {
    }

    const char* InspectorPanel::getTitle() const
    {
        return "Inspector";
    }

    void InspectorPanel::onGui()
    {
        ImGui::Begin(getTitle());

        // 1. Get Selected Entity
        auto selectedIds = m_editor.getSelectedEntities();

        if (selectedIds.size() == 0)
            ImGui::Text("No entity selected.");
        else if (selectedIds.size() == 1)
            drawComponents(*selectedIds.begin());
        else if (selectedIds.size() > 1)
            ImGui::Text("Multiple entities selected. Select a single entity to view details.");

        ImGui::End();
    }

    template <typename T>
    void InspectorPanel::drawComponent(const std::string& name, Entity entity)
    {
        auto& scene = m_engine.scene(); // Assuming this returns entt::registry or similar

        if (scene.hasComponent<T>(entity))
        {
            // Create a collapsible header like Unity
            // ImGuiTreeNodeFlags_DefaultOpen makes it open by default
            bool open = ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

            if (open)
            {
                // Call your existing ComponentUI draw functions
                ComponentUI<T>::draw(scene, entity);
                ImGui::Spacing();
            }
        }
    }

    void InspectorPanel::drawComponents(Entity entity)
    {
        auto& scene = m_engine.scene();

        // Validate entity is still valid
        if (!scene.valid(entity))
        {
            m_editor.deselectEntity();
            return;
        }

        // --- Draw Name (Usually at top, no header) ---
        if (scene.hasComponent<Name>(entity))
        {
            ComponentUI<Name>::draw(scene, entity);
            ImGui::Separator();
        }

        // --- Draw Standard Components ---
        drawComponent<Transform>("Transform", entity);
        drawComponent<Renderable>("Mesh Renderer", entity);
        drawComponent<RigidBody>("Rigid Body", entity);

        // --- "Add Component" Button at the bottom ---
        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("Add Component", ImVec2(-1, 0)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            if (ImGui::MenuItem("Rigid Body"))
            {
                if (!scene.hasComponent<RigidBody>(entity))
                    scene.addComponent<RigidBody>(entity); // Add default params
            }
            if (ImGui::MenuItem("Mesh Renderer"))
            {
                if (!scene.hasComponent<Renderable>(entity))
                    scene.addComponent<Renderable>(entity);
            }
            ImGui::EndPopup();
        }
    }
}
