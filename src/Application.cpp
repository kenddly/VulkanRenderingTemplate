#include <vks/Application.hpp>

namespace vks
{
    void Application::tick()
    {
        updateTime();
        onUpdate(m_deltaTime);
    }

    void Application::updateTime()
    {
        using clock = std::chrono::high_resolution_clock;

        auto now = clock::now();

        if (m_lastFrame.time_since_epoch().count() == 0)
        {
            m_lastFrame = now;
            m_deltaTime = 0.0f;
            return;
        }

        std::chrono::duration<float> delta = now - m_lastFrame;

        Time::setDeltaTime(delta.count());
        Time::totalTime += m_deltaTime;

        m_lastFrame = now;
    }
}
