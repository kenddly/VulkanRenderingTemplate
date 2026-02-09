#pragma once

#include "../core/Singleton.hpp"

namespace vks
{
    class Physics : public Singleton<Physics>
    {
    public:
        static void calculateGravity();
    };
}
