#pragma once

#include "IEditorPanel.hpp"
#include <filesystem>

namespace vks {

    // Forward declaration
    class EditorResourceManager;

    class AssetBrowserPanel : public IEditorPanel {
    public:
        AssetBrowserPanel(Engine& engine);
        ~AssetBrowserPanel() override = default;

        const char* getTitle() const override;
        void onGui() override;
        void handleInput();

    private:
        // Helper to draw the navigation bar (Back button, breadcrumbs)
        void renderTopBar();

        // Helper to draw the grid of icons
        void renderContentGrid();

        std::filesystem::path m_rootPath;
        std::filesystem::path m_currentPath;
        std::filesystem::path m_selectedPath;

        // Visual settings
        float m_padding = 16.0f;
        float m_thumbnailSize = 64.0f;

        const float m_maxThumbnailSize = 512.0f;
        const float m_minThumbnailSize = 16.0f;
    };
}