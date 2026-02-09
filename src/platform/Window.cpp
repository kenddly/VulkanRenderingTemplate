#include <../include/platform/Window.hpp>

#include <../include/gfx/Instance.hpp>

#include <iostream>

using namespace vks;

Window::Window(const glm::ivec2& dimensions, const std::string& title,
    const Instance& instance)
    : m_dimensions(dimensions), m_title(title), m_instance(instance),
    m_surface(VK_NULL_HANDLE), m_framebufferResized(true),
    m_drawFrameFunc([](bool&, float) {}) {

    m_windowInstance = this;
    
    m_window = glfwCreateWindow(dimensions.x, dimensions.y, title.c_str(),
        nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);

    if (glfwCreateWindowSurface(instance.handle(), m_window, nullptr,
        &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create window surface");
    }

    // Attach input system
    m_input.attachToWindow(m_window);
}

Window::~Window() {
    // Clear the user pointer so callbacks can't touch this object after we start tearing down
    if (m_window) {
        glfwSetWindowUserPointer(m_window, nullptr);
    }

    vkDestroySurfaceKHR(m_instance.handle(), m_surface, nullptr);
    glfwDestroyWindow(m_window);
}

void Window::mainLoop() {
    float lastTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();

        // Update time
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        m_drawFrameFunc(m_framebufferResized, deltaTime);
        m_input.update();
    }
}

void Window::GetRequiredExtensions(std::vector<const char*>& out) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    out.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

Window* Window::GetInstance()
{
    return m_windowInstance;
}

void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    if (!window) return;

    // Get the Window instance safely
    void* ptr = glfwGetWindowUserPointer(window);
    if (!ptr) return; // user pointer cleared or not set yet

    Window::GetInstance()->m_framebufferResized = true;
}
