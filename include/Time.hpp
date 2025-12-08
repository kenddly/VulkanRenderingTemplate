#pragma once
#include "Singleton.hpp"

namespace vks
{
    class Time : public Singleton<Time>
    {
    public:
        static float getDeltaTime() { return deltaTime; }
        static float getTotalTime() { return totalTime; }
    private:
        friend class Application;
        inline static double deltaTime = 0.0f; // Time between frames
        inline static double totalTime = 0.0f; // Total elapsed time
    };
}
