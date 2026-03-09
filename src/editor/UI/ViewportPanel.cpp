#include <algorithm>
#include <editor/UI/ViewportPanel.hpp>

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan_core.h>

#include <app/Engine.hpp>

#include "scene/Camera.hpp"

namespace vks
{
    ViewportPanel::ViewportPanel(Engine& engine)
        : IEditorPanel(engine), m_gizmo(engine)
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

        auto proj = m_engine.camera().getProjectionMatrix();
        auto viewProj = proj * m_engine.camera().getViewMatrix();

        m_engine.physics().getDebugRenderer().render(
            m_engine.physics().getJoltSystem(),
            viewProj,
            ImGui::GetWindowDrawList(),
            {avail.x, avail.y},
            {imageScreenPos.x, imageScreenPos.y}
        );
        m_gizmo.onGui(imageScreenPos, avail);

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

        bool allowInput = m_isCameraCaptured || (isFocused && isHovered) && !m_gizmo.isUsing();

        m_engine.editor().allowViewportInput(allowInput);

        // Calculate local coordinates relative to the image
        float localX = mouseScreenPos.x - imageScreenPos.x;
        float localY = mouseScreenPos.y - imageScreenPos.y;

        // Send the exact pixel coordinate to engine system
        m_engine.editor().viewportMousePos = {localX, localY};
    }
}
