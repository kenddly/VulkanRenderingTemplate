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

    void drawImguiEditor();
private:
    uint64_t getSortKey() const;

    void updateTransform();
};