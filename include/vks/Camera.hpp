#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vks/Input.hpp>

namespace vks {

// UBO for camera (matches sphere_mesh.vert, Set 0)
struct CameraUBO
{
    glm::mat4 view;
    glm::mat4 proj;
};


class Camera {
public:
    Camera() = default;

    void init(Input* input, float aspectRatio)
    {
        m_input = input;
        m_aspectRatio = aspectRatio;

        updateMatrices();
    }

    // --------------------------------------------------------------------
    // UPDATE
    // --------------------------------------------------------------------
    void update(float dt)
    {
        if (!m_input) return;

        // Mouse look
        glm::vec2 delta = m_input->mouseDelta();
        if (m_input->isMouseHeld(GLFW_MOUSE_BUTTON_RIGHT)) {
            m_yaw   -= delta.x * mouseSensitivity;
            m_pitch -= delta.y * mouseSensitivity;

            // Clamp pitch (avoid gimbal lock)
            const float limit = glm::radians(89.0f);
            m_pitch = glm::clamp(m_pitch, -limit, limit);
        }

        // Movement controls
        glm::vec3 f = forward();
        glm::vec3 r = right();
        glm::vec3 u = up();   // Z-up

        if (m_input->isKeyHeld(GLFW_KEY_W)) position += f * speed * dt;
        if (m_input->isKeyHeld(GLFW_KEY_S)) position -= f * speed * dt;
        if (m_input->isKeyHeld(GLFW_KEY_D)) position += r * speed * dt;
        if (m_input->isKeyHeld(GLFW_KEY_A)) position -= r * speed * dt;

        if (m_input->isKeyHeld(GLFW_KEY_E)) position += u * speed * dt;
        if (m_input->isKeyHeld(GLFW_KEY_Q)) position -= u * speed * dt;

        updateMatrices();
    }

    // --------------------------------------------------------------------
    // MATRICES
    // --------------------------------------------------------------------
    const glm::mat4& view() const { return m_view; }
    const glm::mat4& proj() const { return m_proj; }

    void setAspect(float aspect)
    {
        m_aspectRatio = aspect;
        updateMatrices();
    }

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
        // Calculate the direction we are looking based on Yaw/Pitch
        glm::vec3 f = forward();

        // Calculate the View Matrix
        m_view = glm::lookAt(
            position,
            position + f,
            glm::vec3(0, 0, 1)
        );

        // Calculate Projection Matrix
        m_proj = glm::perspective(
            glm::radians(45.0f),
            m_aspectRatio,
            0.1f,
            100.0f
        );

        // Flip Y for Vulkan (Clip space Y is inverted compared to OpenGL)
        m_proj[1][1] *= -1;

    }
};
} // namespace vks
