#include <algorithm>
#include <editor/UI/ViewportPanel.hpp>

#include <imgui.h>
#include <vulkan/vulkan_core.h>

#include <app/Engine.hpp>

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

        handleInput();

        // Available region inside the window
        ImVec2 avail = ImGui::GetContentRegionAvail();

        // Clamp (critical on first frame & when minimized)
        avail.x = std::max(avail.x, 1.0f);
        avail.y = std::max(avail.y, 1.0f);

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
        ImGui::PopStyleVar();

        ImGui::End();
    }

    void ViewportPanel::handleInput()
    {
        bool isHovered = ImGui::IsWindowHovered();
        bool isFocused = ImGui::IsWindowFocused();

        if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            m_isCameraCaptured = true;

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Right))
            m_isCameraCaptured = false;

        bool allowInput = m_isCameraCaptured || (isFocused && isHovered);

        m_engine.editor().allowViewportInput(allowInput);

        ImVec2 avail = ImGui::GetContentRegionAvail();
        avail.x = std::max(avail.x, 1.0f);
        avail.y = std::max(avail.y, 1.0f);

        // Get the exact absolute screen coordinate where the image starts
        ImVec2 imageScreenPos = ImGui::GetCursorScreenPos();

        // Get the global mouse position from ImGui
        ImVec2 mouseScreenPos = ImGui::GetMousePos();

        // Calculate local coordinates relative to the image
        float localX = mouseScreenPos.x - imageScreenPos.x;
        float localY = mouseScreenPos.y - imageScreenPos.y;

        // Send the exact pixel coordinate to your engine/picking system
        glm::vec2 pickingPos = {localX, localY};
        m_engine.editor().viewportMousePos = pickingPos;
    }
}
