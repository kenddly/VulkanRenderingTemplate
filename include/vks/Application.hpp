#pragma once
#include <chrono>

#include "Time.hpp"

namespace vks {

    class Engine;

    class Application {
    public:
        virtual ~Application() = default;

        // Called once at startup
        virtual void onInit(Engine& engine) = 0;

        // Called every frame
        void tick();

        // Called every frame for UI
        virtual void onImGui() = 0;

    protected:
        void updateTime();
        virtual void onUpdate(float deltaTime) = 0;

    private:
        std::chrono::high_resolution_clock::time_point m_lastFrame{};
        float m_deltaTime = 0.0f;
        float m_totalTime = 0.0f;
    };
}
