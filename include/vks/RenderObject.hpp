#pragma once
#include <vks/Material.hpp>

class RenderObject
{
public:
    RenderObject();

    vks::Model* model;
    vks::Material* material;
    glm::mat4 transform;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);

    float mass = 1.0;

    void drawImguiEditor();
    void updateTransform();
private:
    uint64_t getSortKey() const;
};