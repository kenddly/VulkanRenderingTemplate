#pragma once

#include "Singleton.hpp"

namespace vks
{
    class Physics : public Singleton<Physics>
    {
    public:
        static void calculateGravity();
    };
}
