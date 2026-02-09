#include <../include/platform/Input.hpp>

using namespace vks;

Input::Input() {}

void Input::attachToWindow(GLFWwindow* window) {
    m_window = window;
    glfwSetWindowUserPointer(window, this);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
}

void Input::update() {
    // clear one-frame states
    for (auto& [k, s] : keys) {
        s.pressed = false;
        s.released = false;
    }
    for (auto& [b, s] : mouseButtons) {
        s.pressed = false;
        s.released = false;
    }
    m_mouseDelta = glm::vec2(0);
    m_scrollDelta = glm::vec2(0);
}

void Input::setCursorState(CursorState state) {
    if (m_window) {
        glfwSetInputMode(m_window, GLFW_CURSOR, state);
    }
}

// ----------------------- Static Callbacks ------------------------------

void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (!input) return;

    auto& s = input->keys[key];

    if (action == GLFW_PRESS) {
        s.pressed = true;
        s.held = true;
    }
    else if (action == GLFW_RELEASE) {
        s.released = true;
        s.held = false;
    }

    if (input->onKeyChanged)
        input->onKeyChanged(key, action);
}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (!input) return;

    
    auto& s = input->mouseButtons[static_cast<MouseButton>(button)];

    if (action == GLFW_PRESS) {
        s.pressed = true;
        s.held = true;
    }
    else if (action == GLFW_RELEASE) {
        s.released = true;
        s.held = false;
    }

    if (input->onMouseButtonChanged)
        input->onMouseButtonChanged(button, action);
}

void Input::CursorPosCallback(GLFWwindow* window, double x, double y) {
    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (!input) return;

    glm::vec2 newPos = {x, y};
    input->m_mouseDelta = newPos - input->m_mousePos;
    input->m_mousePos = newPos;

    if (input->onMouseMove)
        input->onMouseMove(newPos);
}

void Input::ScrollCallback(GLFWwindow* window, double xoff, double yoff) {
    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (!input) return;

    input->m_scrollDelta = {xoff, yoff};

    if (input->onScroll)
        input->onScroll(input->m_scrollDelta);
}
