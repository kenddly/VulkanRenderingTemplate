#pragma once

#include <imgui.h>

#include "EditorGizmo.hpp"
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
        EditorGizmo m_gizmo;
        bool m_isCameraCaptured = false;
    };
}
