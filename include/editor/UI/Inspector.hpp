#pragma once
#include <editor/UI/IEditorPanel.hpp>
#include <app/Engine.hpp>
#include <editor/UI/components/Name.hpp>
#include <editor/UI/components/Transform.hpp>
#include <editor/UI/components/Renderable.hpp>
#include <editor/UI/components/RigidBody.hpp>
#include <imgui.h>

namespace vks {

    class InspectorPanel : public IEditorPanel {
    public:
        InspectorPanel(Engine& engine);

        const char* getTitle() const override;

        void onGui() override;

    private:
        Engine& m_engine;
        EngineEditor& m_editor;

        // Helper to draw a specific component with a consistent UI style
        template<typename T>
        void drawComponent(const std::string& name, Entity entity);

        void drawComponents(Entity entity);
    };
}