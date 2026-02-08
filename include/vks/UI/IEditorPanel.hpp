#pragma once

namespace vks {
    class Engine;

    class IEditorPanel {
    public:
        explicit IEditorPanel(Engine& engine) : m_engine(engine) {}
        virtual ~IEditorPanel() = default;

        // The main draw loop for this specific window
        virtual void onGui() = 0;

        // Optional: Toggle visibility
        bool isOpen = true;
        virtual const char* getTitle() const = 0;

    protected:
        Engine& m_engine;
    };
}