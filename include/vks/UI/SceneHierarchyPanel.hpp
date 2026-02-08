#pragma once
#include <vks/UI/IEditorPanel.hpp>

namespace vks
{
    class SceneHierarchyPanel : public IEditorPanel
    {
    public:
        using IEditorPanel::IEditorPanel;

        const char* getTitle() const override { return "Scene Hierarchy"; }
        void onGui() override;
    };
}
