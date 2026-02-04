#include <vks/Physics.hpp>

#include "Time.hpp"
#include "vks/Application.hpp"
#include "vks/RenderObject.hpp"

namespace vks
{
    void Physics::calculateGravity()
    {
        // auto& objects = Application::getInstance()->getRenderObjects();
        //
        // const float G = 0.1f; // tune this (too high = objects explode)
        // float dt = Time::getDeltaTime();
        //
        // // 1. Reset accelerations
        // for (auto& pair : objects)
        // {
        //     RenderObject& obj = pair.second;
        //     obj.acceleration = glm::vec3(0.0f);
        // }
        //
        // // 2. Compute pairwise gravitational forces
        // for (auto itA = objects.begin(); itA != objects.end(); ++itA)
        // {
        //     RenderObject& A = itA->second;
        //
        //     for (auto itB = std::next(itA); itB != objects.end(); ++itB)
        //     {
        //         RenderObject& B = itB->second;
        //
        //         glm::vec3 dir = B.position - A.position;
        //         float dist = glm::length(dir);
        //
        //         if (dist < 0.0001f)
        //             continue; // avoid division by zero
        //
        //         glm::vec3 n = dir / dist; // normalized direction
        //         float force = G * (A.mass * B.mass) / (dist * dist);
        //
        //         glm::vec3 accelA = (force / A.mass) * n;
        //         glm::vec3 accelB = -(force / B.mass) * n;
        //
        //         A.acceleration += accelA;
        //         B.acceleration += accelB;
        //     }
        // }
        //
        // // 3. Integrate velocity + position
        // for (auto& pair : objects)
        // {
        //     RenderObject& obj = pair.second;
        //
        //     obj.velocity += obj.acceleration * dt;
        //     obj.position += obj.velocity * dt;
        //
        //     obj.updateTransform();
        // }
    }
}
