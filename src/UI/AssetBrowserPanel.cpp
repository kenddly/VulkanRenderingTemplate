#include <vks/UI/AssetBrowserPanel.hpp>
#include <vks/Engine.hpp>

#include <vector>
#include <algorithm>
#include <iostream>
#include <imgui.h>

namespace vks {

    AssetBrowserPanel::AssetBrowserPanel(Engine& engine)
        : IEditorPanel(engine)
    {
        // Set the starting directory
        // Ensure this directory exists in your project root!
        m_rootPath = "assets";
        m_currentPath = m_rootPath;
    }

    const char* AssetBrowserPanel::getTitle() const {
        return "Asset Browser";
    }

    void AssetBrowserPanel::onGui() {
        if (!isOpen) return;

        ImGui::Begin(getTitle(), &isOpen);
        handleInput();

        renderTopBar();
        ImGui::Separator();
        renderContentGrid();

        ImGui::End();
    }

    void AssetBrowserPanel::handleInput() {
        // Only handle input if the window is currently hovered or focused
        if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && !ImGui::IsWindowFocused()) {
            return;
        }

        ImGuiIO& io = ImGui::GetIO();

        // Check if CTRL is held down
        if (io.KeyCtrl) {
            float zoomDelta = 0.0f;

            // 1. Mouse Wheel Zoom
            if (io.MouseWheel != 0.0f) {
                zoomDelta += io.MouseWheel * 10.0f; // Speed factor
            }

            // 2. Keyboard Zoom (+ and -)
            // Support both main keyboard and numpad
            if (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) {
                zoomDelta += 10.0f;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) {
                zoomDelta -= 10.0f;
            }

            // Apply zoom
            if (zoomDelta != 0.0f) {
                m_thumbnailSize += zoomDelta;

                // Clamp values to prevent icons becoming invisible or too massive
                if (m_thumbnailSize < m_minThumbnailSize) m_thumbnailSize = m_minThumbnailSize;
                if (m_thumbnailSize > m_maxThumbnailSize) m_thumbnailSize = m_maxThumbnailSize;
            }
        }
    }

    void AssetBrowserPanel::renderTopBar() {
        // Only show back button if we aren't at the root
        if (m_currentPath != m_rootPath) {
            if (ImGui::Button("<- Back")) {
                m_currentPath = m_currentPath.parent_path();
            }
            ImGui::SameLine();
        }

        // Show current path text
        ImGui::Text("Path: %s", m_currentPath.string().c_str());
    }

    void AssetBrowserPanel::renderContentGrid() {
        // Calculate Grid Dimensions
        float cellSize = m_thumbnailSize + m_padding;
        float panelWidth = ImGui::GetContentRegionAvail().x;

        // Prevent division by zero or negative columns
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        if (ImGui::BeginTable("AssetBrowserTable", columnCount)) {

            // Collect Directory Entries
            std::vector<std::filesystem::directory_entry> entries;
            try {
                for (const auto& entry : std::filesystem::directory_iterator(m_currentPath)) {
                    entries.push_back(entry);
                }
            } catch (const std::filesystem::filesystem_error& e) {
                ImGui::TextColored({1, 0, 0, 1}, "Error accessing directory: %s", e.what());
                ImGui::EndTable();
                return;
            }

            // Sort (Directories First, then Alphabetical)
            std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
                if (a.is_directory() != b.is_directory()) {
                    return a.is_directory() > b.is_directory();
                }
                return a.path().filename() < b.path().filename();
            });

            // Render Items
            for (const auto& entry : entries) {
                ImGui::TableNextColumn();

                const auto& path = entry.path();
                std::string filename = path.filename().string();

                // Push ID to avoid ImGui collisions between items with same name in diff folders
                ImGui::PushID(filename.c_str());

                // --- A. Icon Selection ---
                // Ask the resource manager for the correct icon based on file type
                ImTextureID iconId = EditorResourceManager::Instance().getIcon(path);

                // --- B. Draw Icon Button ---
                // Make button background transparent so it looks like just an icon
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

                // ImageButton returns true on Click
                if (ImGui::ImageButton("##icon", iconId, {m_thumbnailSize, m_thumbnailSize}, {0, 0}, {1, 1})) {
                    // Single Click: Select
                    if (!entry.is_directory()) {
                        m_selectedPath = path;
                    }
                }

                ImGui::PopStyleColor();

                // --- C. Drag and Drop Source ---
                if (ImGui::BeginDragDropSource()) {
                    std::string itemPath = path.string();

                    // Payload tag "CONTENT_BROWSER_ITEM" - Receivers must look for this
                    // We pass the string path including null terminator
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), itemPath.length() + 1);

                    // Render a preview tooltip
                    ImGui::Image(iconId, {32, 32});
                    ImGui::Text("%s", filename.c_str());

                    ImGui::EndDragDropSource();
                }

                // --- D. Double Click Navigation ---
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    if (entry.is_directory()) {
                        m_currentPath = path;
                        // Reset selection when changing folders
                        m_selectedPath.clear();
                    } else {
                        // Optional: Open file logic here
                        std::cout << "Opening file: " << path << std::endl;
                    }
                }

                // --- E. File Name Text ---
                // Calculate position to center the text below the icon
                float textWidth = ImGui::CalcTextSize(filename.c_str()).x;
                if (textWidth < cellSize) {
                    float indent = (cellSize - textWidth) * 0.5f;
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
                }

                // Highlight text if selected
                if (m_selectedPath == path) {
                    ImGui::TextColored({0.4f, 0.8f, 1.0f, 1.0f}, "%s", filename.c_str());
                } else {
                    ImGui::TextWrapped("%s", filename.c_str());
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }
}