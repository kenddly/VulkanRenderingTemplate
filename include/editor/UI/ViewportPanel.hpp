#pragma once

#include <imgui.h>
#include <glm/fwd.hpp>

#include "IEditorPanel.hpp"

namespace vks
{
    class ViewportPanel : public IEditorPanel
    {
    public:
        ViewportPanel(Engine& engine);
        ~ViewportPanel() override = default;

        const char* getTitle() const override;
        void onGui() override;
        void handleInput(ImVec2 imageScreenPos, ImVec2 mouseScreenPos);
    private:
        ImVec2 m_lastViewportSize{0, 0};
        bool m_isCameraCaptured = false;
        bool m_usingGizmo = false;
    };
}
