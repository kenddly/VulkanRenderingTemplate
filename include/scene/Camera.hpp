#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <platform/Input.hpp>

#include <platform/events/EventManager.hpp>
#include <platform/events/Events.hpp>

namespace vks {

// UBO for camera (matches sphere_mesh.vert, Set 0)
struct CameraUBO
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 position;
};


class Camera {
public:
    Camera()
    {
        EventManager::subscribe<ViewportResizeEvent>([this](ViewportResizeEvent e)
        {
            setAspect(float(e.newWidth) / float(e.newHeight));
        });
    }

    void init(Input* input, float aspectRatio)
    {
        m_input = input;
        m_aspectRatio = aspectRatio;

        updateMatrices();
    }

    // --------------------------------------------------------------------
    // UPDATE
    // --------------------------------------------------------------------
    void update(float dt, bool allowInput)
    {
        if (!m_input) return;
        if (!allowInput) {
            m_input->setCursorState(Input::CursorState::NORMAL);
            return;
        }

        // Mouse look
        glm::vec2 delta = m_input->mouseDelta();
        
        if (m_input->isMousePressed(Input::MouseButton::RIGHT))
            m_input->setCursorState(Input::CursorState::DISABLED);
        
        if (m_input->isMouseHeld(Input::MouseButton::RIGHT)) {
            m_yaw   -= delta.x * mouseSensitivity;
            m_pitch -= delta.y * mouseSensitivity;

            // Clamp pitch (avoid gimbal lock)
            const float limit = glm::radians(89.0f);
            m_pitch = glm::clamp(m_pitch, -limit, limit);

        // Movement controls
        glm::vec3 f = forward();
        glm::vec3 r = right();
        glm::vec3 u = up();   // Z-up

        float currentSpeed = speed;
        if (m_input->isKeyHeld(GLFW_KEY_LEFT_SHIFT)) currentSpeed = speed * 5.0f;

        if (m_input->isKeyHeld(GLFW_KEY_W)) position += f * currentSpeed * dt;
        if (m_input->isKeyHeld(GLFW_KEY_S)) position -= f * currentSpeed * dt;
        if (m_input->isKeyHeld(GLFW_KEY_D)) position += r * currentSpeed * dt;
        if (m_input->isKeyHeld(GLFW_KEY_A)) position -= r * currentSpeed * dt;

        if (m_input->isKeyHeld(GLFW_KEY_E)) position += u * currentSpeed * dt;
        if (m_input->isKeyHeld(GLFW_KEY_Q)) position -= u * currentSpeed * dt;

        updateMatrices();
        }

        if (m_input->isMouseReleased(Input::MouseButton::RIGHT))
            m_input->setCursorState(Input::CursorState::NORMAL);
    }

    // --------------------------------------------------------------------
    // MATRICES
    // --------------------------------------------------------------------
    const glm::mat4& view() const { return m_view; }
    const glm::mat4& proj() const { return m_proj; }

    const glm::vec3 getPosition() const { return position; }
    const glm::vec3 getDirection() const { return forward(); }

    void setAspect(float aspect)
    {
        m_aspectRatio = aspect;
        updateMatrices();
    }

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

private:
    // Camera orientation
    float m_yaw   = glm::radians(0.0f);
    float m_pitch = glm::radians(0.0f);

    float speed = 5.0f;
    float mouseSensitivity = 0.002f;

    glm::vec3 position = glm::vec3(5, 5, 5);

    Input* m_input = nullptr;
    float m_aspectRatio = 1.0f;

    glm::mat4 m_view{1.0f};
    glm::mat4 m_proj{1.0f};

private:
    // Directions in Z-UP world
    glm::vec3 forward() const
    {
        return glm::normalize(glm::vec3(
            cosf(m_pitch) * cosf(m_yaw),
            cosf(m_pitch) * sinf(m_yaw),
            sinf(m_pitch)
        ));
    }

    glm::vec3 right() const
    {
        return glm::normalize(glm::cross(forward(), glm::vec3(0,0,1)));
    }

    glm::vec3 up() const
    {
        return glm::normalize(glm::cross(right(), forward()));
    }

    void updateMatrices()
    {
        m_view = getViewMatrix();
        m_proj = getProjectionMatrix();

        // Flip Y for Vulkan (Clip space Y is inverted compared to OpenGL)
        m_proj[1][1] *= -1;

    }
};

inline glm::mat4 Camera::getViewMatrix() const
{
    // Recalculate the direction we are looking based on Yaw/Pitch
    glm::vec3 f = forward();

    // Calculate the View Matrix
    return glm::lookAt(
        position,
        position + f,
        glm::vec3(0, 0, 1)
    );
}

inline glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::perspective(
        glm::radians(45.0f),
        m_aspectRatio,
        1.0f,
        1000.0f
    );
}
} // namespace vks
