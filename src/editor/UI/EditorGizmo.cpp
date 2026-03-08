#include <editor/UI/EditorGizmo.hpp>
#include <editor/UI/components/Transform.hpp>
#include <app/Engine.hpp>

#include <glm/gtc/type_ptr.hpp>

using namespace vks;

EditorGizmo::EditorGizmo(Engine& engine) : m_engine(engine)
{
}

void EditorGizmo::onGui(ImVec2 imageScreenPos, ImVec2 imageSize)
{
    handleShortcuts();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(imageScreenPos.x, imageScreenPos.y, imageSize.x, imageSize.y);

    glm::mat4 view = m_engine.camera().getViewMatrix();
    glm::mat4 proj = m_engine.camera().getProjectionMatrix();

    auto entities = m_engine.editor().getSelectedEntities();
    if (entities.size() != 1) return;

    auto& transform = m_engine.scene().getComponent<Transform>(*entities.begin());
    glm::mat4 modelMatrix = transform.transform; if (m_snap) { static auto translationSnap = new float[3]{1.0f, 1.0f, 1.0f}; static auto rotationSnap = new float[3]{15.0f, 15.0f, 15.0f}; float* snapValue = nullptr; switch (m_operation) { case ImGuizmo::ROTATE: snapValue = rotationSnap; break; case ImGuizmo::TRANSLATE: case ImGuizmo::SCALE: snapValue = translationSnap; break; default: break; } ImGuizmo::Manipulate( glm::value_ptr(view), glm::value_ptr(proj), m_operation, m_mode, glm::value_ptr(modelMatrix), nullptr, snapValue ); } else { ImGuizmo::Manipulate( glm::value_ptr(view), glm::value_ptr(proj), m_operation, m_mode, glm::value_ptr(modelMatrix) ); } m_usingGizmo = ImGuizmo::IsUsing(); if (m_usingGizmo) writeBackTransform(transform, modelMatrix); } void EditorGizmo::handleShortcuts() { if (ImGui::IsKeyPressed(ImGuiKey_W)) m_operation = ImGuizmo::TRANSLATE; if (ImGui::IsKeyPressed(ImGuiKey_E)) m_operation = ImGuizmo::ROTATE; if (ImGui::IsKeyPressed(ImGuiKey_R)) m_operation = ImGuizmo::SCALE; if (ImGui::IsKeyPressed(ImGuiKey_Q)) m_mode = (m_mode == ImGuizmo::LOCAL) ? ImGuizmo::WORLD : ImGuizmo::LOCAL; if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
        m_snap = true;
    else if (m_snap)
        m_snap = false; // Disable snap when shift is released
}

void EditorGizmo::writeBackTransform(Transform& transform, const glm::mat4& manipulated)
{
    static const glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1, -1, 1));
    glm::mat4 cleanTRS = flipY * manipulated * flipY;

    glm::vec3 translation, rotationDeg, scale;
    ImGuizmo::DecomposeMatrixToComponents(
        glm::value_ptr(cleanTRS),
        glm::value_ptr(translation),
        glm::value_ptr(rotationDeg),
        glm::value_ptr(scale)
    );

    transform.position = translation;
    transform.rotation = glm::radians(rotationDeg);
    transform.scale = scale;
    transform.updateTransform();
}
