#pragma once

#include <imgui.h>

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
        void handleInput();
    private:
        ImVec2 m_lastViewportSize{0, 0};
        bool m_isCameraCaptured = false;
    };
}
