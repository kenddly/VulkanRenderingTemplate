#pragma once
#include <vector>
#include <memory>
#include <editor/UI/IEditorPanel.hpp>
#include <editor/UI/EditorResourceManager.hpp>

#include <gfx/Descriptors.hpp>

namespace vks {
    class Engine;

    class EngineEditor {
    public:
        EngineEditor(Engine& engine);

        void onInit();
        void onGui() const;

    private:
        // Helper to register panels easily
        template <typename T> void addPanel();

        Engine& m_engine;
        std::vector<std::unique_ptr<IEditorPanel>> m_panels;

        Ref<EditorResourceManager> resourceManager;
    };
}
