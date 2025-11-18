#pragma once


#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <functional>
#include <unordered_map>

namespace vks
{
class Window;

class Input {
public:
    struct KeyState {
        bool pressed = false;
        bool released = false;
        bool held = false;
    };

    struct MouseButtonState {
        bool pressed = false;
        bool released = false;
        bool held = false;
    };

    // ---- PUBLIC API -------------------------------------------------------------------

    void update(); // call once per frame (clears pressed/released states)

    bool isKeyPressed(int key) const    { try { return keys.at(key).pressed; } catch (std::exception _) { return false; }}
    bool isKeyReleased(int key) const   { try {return keys.at(key).released; } catch (std::exception _) { return false; }}
    bool isKeyHeld(int key) const       { try {return keys.at(key).held; } catch (std::exception _) { return false; }}

    bool isMousePressed(int btn) const  { try {return mouseButtons.at(btn).pressed; } catch (std::exception _) { return false; }}
    bool isMouseReleased(int btn) const { try {return mouseButtons.at(btn).released; } catch (std::exception _) { return false; }}
    bool isMouseHeld(int btn) const     { try {return mouseButtons.at(btn).held; } catch (std::exception _) { return false; }}

    glm::vec2 mousePos() const { return m_mousePos; }
    glm::vec2 mouseDelta() const { return m_mouseDelta; }
    glm::vec2 scrollDelta() const { return m_scrollDelta; }

    // ---- CALLBACKS THAT USER CAN BIND -------------------------------------------------
    std::function<void(int, int)> onKeyChanged;             // key, action
    std::function<void(int, int)> onMouseButtonChanged;     // button, action
    std::function<void(const glm::vec2&)> onMouseMove;
    std::function<void(const glm::vec2&)> onScroll;

    friend class Window;
private:

    Input();

    static void KeyCallback(GLFWwindow*, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow*, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow*, double x, double y);
    static void ScrollCallback(GLFWwindow*, double xoff, double yoff);

    void attachToWindow(GLFWwindow* window);

private:
    std::unordered_map<int, KeyState> keys;
    std::unordered_map<int, MouseButtonState> mouseButtons;

    glm::vec2 m_mousePos = {0, 0};
    glm::vec2 m_mouseLastPos = {0, 0};
    glm::vec2 m_mouseDelta = {0, 0};
    glm::vec2 m_scrollDelta = {0, 0};

    GLFWwindow* m_window = nullptr;
};
}