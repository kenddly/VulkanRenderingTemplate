#include <algorithm>
#include <editor/UI/ViewportPanel.hpp>

#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan_core.h>

#include <app/Engine.hpp>

#include "scene/Camera.hpp"

namespace vks
{
    ViewportPanel::ViewportPanel(Engine& engine)
        : IEditorPanel(engine)
    {
    }

    const char* ViewportPanel::getTitle() const
    {
        return "Viewport";
    }

    void ViewportPanel::onGui()
    {
        if (!isOpen)
            return;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin(getTitle(), &isOpen,
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);

        // Available region inside the window
        ImVec2 avail = ImGui::GetContentRegionAvail();

        // Clamp (critical on first frame & when minimized)
        avail.x = std::max(avail.x, 1.0f);
        avail.y = std::max(avail.y, 1.0f);

        // Get the top-left corner of the image in screen coordinates
        ImVec2 imageScreenPos = ImGui::GetCursorScreenPos();
        // Get the global mouse position from ImGui
        ImVec2 mouseScreenPos = ImGui::GetMousePos();


        VkExtent2D currentExtent = m_engine.getRenderTarget()->extent();
        if (currentExtent.width != static_cast<uint32_t>(avail.x) ||
            currentExtent.height != static_cast<uint32_t>(avail.y))
        {
            EventManager::emit<ViewportResizeEvent>(ViewportResizeEvent{
                static_cast<int>(avail.x), static_cast<int>(avail.y)
            });
        }

        uint32_t frameIndex = m_engine.renderer().getCurrentFrameIndex();
        auto viewportTexture = m_engine.getRenderTarget()->renderTargetImage(frameIndex);

        // Draw the image
        ImGui::Image(
            viewportTexture,
            avail,
            ImVec2(0, 0),
            ImVec2(1, 1)
        );

        // --- ImGuizmo: must happen after Image but before End ---
        ImGuizmo::SetDrawlist(); // draws into the current window's drawlist

        // Tell ImGuizmo the screen-space rect of the viewport
        ImGuizmo::SetRect(
            imageScreenPos.x, imageScreenPos.y,
            avail.x, avail.y
        );

        // Rotation that converts Z-up to Y-up for ImGuizmo
        static const glm::mat4 zUpToYUp = glm::rotate(
            glm::mat4(1.0f),
            glm::radians(-90.0f),
            glm::vec3(1, 0, 0)
        );
        static const glm::mat4 yUpToZUp = glm::inverse(zUpToYUp);

        // Fetch view & projection matrices from your camera
        // These must be column-major float[16]
        glm::mat4 view = m_engine.camera().getViewMatrix();
        glm::mat4 proj = m_engine.camera().getProjectionMatrix();

        // Model matrix of the selected entity
        auto entities = m_engine.editor().getSelectedEntities();
        if (entities.size() == 1)
        {
            auto selectedEntity = *entities.begin();

            auto& transform = m_engine.scene().getComponent<Transform>(selectedEntity);
            auto modelMatrix = transform.transform;

            ImGuizmo::Manipulate(
                glm::value_ptr(view),
                glm::value_ptr(proj),
                ImGuizmo::TRANSLATE, // or ROTATE, SCALE, UNIVERSAL
                ImGuizmo::LOCAL, // or WORLD
                glm::value_ptr(modelMatrix)
            );

            // If the gizmo was used, write back the new transform
            if (ImGuizmo::IsUsing())
            {
                m_usingGizmo = true;

                static const glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1, -1, 1));
                glm::mat4 cleanTRS = flipY * modelMatrix * flipY;

                glm::vec3 translation, rotation, scale;
                ImGuizmo::DecomposeMatrixToComponents(
                    glm::value_ptr(cleanTRS),
                    glm::value_ptr(translation),
                    glm::value_ptr(rotation),
                    glm::value_ptr(scale)
                );
                transform.position = translation;
                transform.rotation = glm::radians(rotation);
                transform.scale = scale;
                transform.updateTransform();
            }
            else
            {
                m_usingGizmo = false;
            }
        }

        handleInput(imageScreenPos, mouseScreenPos);

        ImGui::PopStyleVar();
        ImGui::End();
    }

    void ViewportPanel::handleInput(ImVec2 imageScreenPos, ImVec2 mouseScreenPos)
    {
        bool isHovered = ImGui::IsWindowHovered();
        bool isFocused = ImGui::IsWindowFocused();

        if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            m_isCameraCaptured = true;

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Right))
            m_isCameraCaptured = false;

        bool allowInput = m_isCameraCaptured || (isFocused && isHovered) && !m_usingGizmo;

        m_engine.editor().allowViewportInput(allowInput);

        // Calculate local coordinates relative to the image
        float localX = mouseScreenPos.x - imageScreenPos.x;
        float localY = mouseScreenPos.y - imageScreenPos.y;

        // Send the exact pixel coordinate to your engine/picking system
        glm::vec2 pickingPos = {localX, localY};
        m_engine.editor().viewportMousePos = pickingPos;
    }
}
