#include <../include/vks/UI/EngineEditor.hpp>
#include "vks/Engine.hpp"

#include <imgui.h>
#include <vector>
#include <vks/UI/AssetBrowserPanel.hpp>
#include <vks/UI/SceneHierarchyPanel.hpp>

namespace vks
{
    EngineEditor::EngineEditor(Engine& engine) : m_engine(engine)
    {
    }

    void EngineEditor::onInit()
    {
        // Register Panels
        addPanel<SceneHierarchyPanel>();
        addPanel<AssetBrowserPanel>();

        resourceManager = std::make_shared<EditorResourceManager>(m_engine.device());
    }

    void EngineEditor::onGui() const
    {
        // 1. Draw Main Menu Bar (Global)
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("View"))
            {
                for (auto& panel : m_panels)
                {
                    if (ImGui::MenuItem(panel->getTitle(), nullptr, panel->isOpen))
                    {
                        panel->isOpen = !panel->isOpen;
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // 2. Iterate and draw all registered panels
        for (auto& panel : m_panels)
        {
            panel->onGui();
        }
    }

    // Helper to register panels easily
    template <typename T>
    void EngineEditor::addPanel()
    {
        // Create the panel and pass the engine reference to it
        m_panels.push_back(std::make_unique<T>(m_engine));
    }
}
