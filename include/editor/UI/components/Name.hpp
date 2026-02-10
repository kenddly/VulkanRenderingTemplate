#pragma once

#include <cstring>
#include <string>
#include <scene/Components.hpp>
#include <editor/UI/ComponentsUI.hpp>

namespace vks
{
    template<>
    struct ComponentUI<Name>
    {
        static void draw(Scene& scene, Entity entity)
        {
            auto& name = scene.getComponent<Name>(entity);
            char buffer[256];
            std::strncpy(buffer, name.value.c_str(), sizeof(buffer));
            if (ImGui::InputText("##Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                name.value = buffer;
            }
        }
    };
}