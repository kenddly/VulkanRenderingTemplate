#pragma once
#include <vector>
#include <memory>
#include <vks/UI/IEditorPanel.hpp>
#include <vks/UI/EditorResourceManager.hpp>

#include "vks/Descriptors.hpp"

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
