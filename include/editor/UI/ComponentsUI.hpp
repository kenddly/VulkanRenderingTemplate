#pragma once

#include <scene/Scene.hpp>

namespace vks
{
    template <typename T>
    struct ComponentUI
    {
        static void draw(Scene&, entt::entity);
    };

}