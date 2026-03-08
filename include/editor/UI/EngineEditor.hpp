#pragma once
#include <vector>
#include <memory>
#include <editor/UI/IEditorPanel.hpp>
#include <editor/UI/EditorResourceManager.hpp>

#include "scene/Scene.hpp"

namespace vks {
    class Engine;

    class EngineEditor
    {
    public:
        EngineEditor(Engine& engine);

        void onInit();
        void onGui();
        void selectEntity(Entity entityId) { m_selectedEntities.clear(); addSelectedEntity(entityId); }
        void addSelectedEntity(Entity entityId) { m_selectedEntities.emplace(entityId); }
        void deselectEntity() { m_selectedEntities.clear(); }

        std::unordered_set<Entity> getSelectedEntities() { return m_selectedEntities; }

        void allowViewportInput(bool allow) { m_allowViewportInput = allow; }
        bool isViewportInputAllowed() const { return m_allowViewportInput; }

        glm::vec2 viewportMousePos{};
    private:
        std::unordered_set<Entity> m_selectedEntities{};
        bool m_allowViewportInput = false;

        // Helper to register panels easily
        template <typename T> void addPanel();

        Engine& m_engine;
        std::vector<std::unique_ptr<IEditorPanel>> m_panels;

        Ref<EditorResourceManager> resourceManager;
    };
}
