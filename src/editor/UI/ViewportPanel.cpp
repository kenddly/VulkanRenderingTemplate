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

        ImGui::Begin(getTitle(), &isOpen,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);

        // Available region inside the window
        ImVec2 avail = ImGui::GetContentRegionAvail();

        VkExtent2D currentExtent = m_engine.getRenderTarget()->extent();
        if (currentExtent.width != static_cast<uint32_t>(avail.x) ||
            currentExtent.height != static_cast<uint32_t>(avail.y))
        {
            EventManager::emit<ViewportResizeEvent>(ViewportResizeEvent{static_cast<int>(avail.x), static_cast<int>(avail.y)});
        }


        // Clamp (critical on first frame & when minimized)
        avail.x = std::max(avail.x, 1.0f);
        avail.y = std::max(avail.y, 1.0f);


        uint32_t frameIndex = m_engine.renderer().getCurrentFrameIndex();
        auto viewportTexture = m_engine.getRenderTarget()->renderTargetImage(frameIndex);

        // Draw the image
        ImGui::Image(
            viewportTexture,
            avail,
            ImVec2(0, 0),
            ImVec2(1, 1)
        );

        handleInput();

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
    }
}
