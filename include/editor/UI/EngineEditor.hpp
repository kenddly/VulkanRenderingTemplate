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
        void onGui() const;
        void selectEntity(Entity entityId) { m_selectedEntity = entityId; }
        void deselectEntity() { m_selectedEntity = entt::null; }

        Entity getSelectedEntity() { return m_selectedEntity; }

        void allowViewportInput(bool allow) { m_allowViewportInput = allow; }
        bool isViewportInputAllowed() const { return m_allowViewportInput; }

        glm::vec2 viewportMousePos;
    private:
        Entity m_selectedEntity = entt::null;
        bool m_allowViewportInput;

        // Helper to register panels easily
        template <typename T> void addPanel();

        Engine& m_engine;
        std::vector<std::unique_ptr<IEditorPanel>> m_panels;

        Ref<EditorResourceManager> resourceManager;
    };
}
