#pragma once

#include <string>
#include <core/types.hpp>

#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace vks
{
    class Material;
    class Model;

    struct Name
    {
        std::string value;
    };

    struct Transform
    {
        glm::vec3 position{0.0f};
        glm::vec3 rotation{0.0f};
        glm::vec3 scale{1.0f};

        glm::mat4 transform{1.0f};

        void updateTransform()
        {
            // Convert rotation to quaternion
            auto qRotation = glm::quat(rotation);

            // Construct left-handed transformation matrix
            glm::mat4 leftTransform = glm::translate(glm::mat4(1.0f), position) *
                glm::mat4(qRotation) *
                glm::scale(glm::mat4(1.0f), scale);

            // Construct right-handed conversion matrix
            glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
            // Convert left-handed matrix to right-handed
            transform = flipY * leftTransform * flipY;
        }
    };

    struct Renderable
    {
        Ref<Model> model;
        Ref<Material> material;

        void drawImGuiEditor()
        {
        }
    };

    struct RigidBody
    {
        glm::vec3 velocity{0.0f};
        glm::vec3 acceleration{0.0f};
        float mass = 1.0f;
    };
}
