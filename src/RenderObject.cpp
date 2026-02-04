#include <vks/RenderObject.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

RenderObject::RenderObject() : model(nullptr), material(nullptr), transform()
{
}

uint64_t RenderObject::getSortKey() const
{
    return material->layer_priority;
}

void RenderObject::drawImguiEditor()
{
    if (ImGui::TreeNode("Transform"))
    {
        ImGui::Text("Transform");
        bool c = false;
        c |= ImGui::DragFloat3("Position", &position.x, 0.1);
        c |= ImGui::DragFloat3("Rotation", &rotation.x, 0.1);
        c |= ImGui::DragFloat3("Scale", &scale.x, 0.1);

        ImGui::Separator();
        c |= ImGui::DragFloat3("Acceleration", &acceleration.x, 0.1);
        c |= ImGui::DragFloat3("Velocity", &velocity.x, 0.1);
        c |= ImGui::DragFloat("Mass", &mass, 0.1, 0.01, 1000.0);

        if (c) updateTransform();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Material"))
    {
        ImGui::Text("Material: %s", material->getPipelineName().c_str());
        material->drawImguiEditor();
        ImGui::TreePop();
    }
}

void RenderObject::updateTransform()
{
    // Convert rotation to quaternion
    auto qRotation = glm::quat(rotation);

    // Construct left-handed transformation matrix
    glm::mat4 leftTransform = glm::translate(glm::mat4(1.0f), position) *
        glm::mat4(qRotation) *
        glm::scale(glm::mat4(1.0f), scale);

    // Construct right-handed conversion matrix
    glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
    // Convert left-handed matrix to right-handed
    transform = flipY * leftTransform * flipY;
}
