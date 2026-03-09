#pragma once

// ============================================================
//  DebugPanel.hpp
//
//  An IEditorPanel that renders all variables registered in
//  DebugRegistry as an editable ImGui window, grouped by
//  category into collapsible tree nodes.
// ============================================================

#include <editor/UI/IEditorPanel.hpp>
#include <editor/DebugRegistry.hpp>

namespace vks
{
    class DebugPanel : public IEditorPanel
    {
    public:
        DebugPanel(Engine& engine);

        const char* getTitle() const override;
        void onGui() override;

    private:
        void renderVar(const DebugRegistry::DebugVar& v);
    };

} // namespace vks