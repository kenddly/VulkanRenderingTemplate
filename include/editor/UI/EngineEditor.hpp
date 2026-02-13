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
        void selectEntity(int entityId) { m_selectedEntity = entityId; }
        void deselectEntity() { m_selectedEntity = -1; }

        int getSelectedEntity() { return m_selectedEntity; }

    private:
        int m_selectedEntity = -1;

        // Helper to register panels easily
        template <typename T> void addPanel();

        Engine& m_engine;
        std::vector<std::unique_ptr<IEditorPanel>> m_panels;

        Ref<EditorResourceManager> resourceManager;
    };
}
