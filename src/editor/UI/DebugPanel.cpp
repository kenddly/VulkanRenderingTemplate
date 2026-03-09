#include <editor/UI/DebugPanel.hpp>

#include <imgui.h>
#include <glm/glm.hpp>
#include <map>
#include <string>

using namespace vks;

DebugPanel::DebugPanel(Engine& engine) : IEditorPanel(engine)
{
}

const char* DebugPanel::getTitle() const
{
    return "Debug";
}

void DebugPanel::onGui()
{
    ImGui::Begin("Debug Variables");

    const auto& vars = DebugRegistry::get().vars();

    if (vars.empty())
    {
        ImGui::TextDisabled("No debug variables registered.");
        ImGui::End();
        return;
    }

    // Group vars by category
    std::map<std::string, std::vector<const DebugRegistry::DebugVar*>> groups;
    for (const auto& v : vars)
        groups[v.category].push_back(&v);

    for (auto& [category, entries] : groups)
    {
        if (ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Indent(8.f);

            for (const auto* entry : entries)
                renderVar(*entry);

            ImGui::Unindent(8.f);
        }
    }

    ImGui::End();
}

void DebugPanel::renderVar(const DebugRegistry::DebugVar& v)
{
    const char* label = v.label.c_str();

    std::visit([&](auto* ptr)
    {
        using T = std::decay_t<decltype(*ptr)>;

        if constexpr (std::is_same_v<T, float>)
        {
            ImGui::DragFloat(label, ptr, 0.01f);
        }
        else if constexpr (std::is_same_v<T, int>)
        {
            ImGui::DragInt(label, ptr);
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            ImGui::Checkbox(label, ptr);
        }
        else if constexpr (std::is_same_v<T, glm::vec2>)
        {
            ImGui::DragFloat2(label, &ptr->x, 0.01f);
        }
        else if constexpr (std::is_same_v<T, glm::vec3>)
        {
            ImGui::DragFloat3(label, &ptr->x, 0.01f);
        }
        else if constexpr (std::is_same_v<T, glm::vec4>)
        {
            ImGui::DragFloat4(label, &ptr->x, 0.01f);
        }
    }, v.ptr);
}
