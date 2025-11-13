#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <vector>
#include <glm/glm.hpp>

namespace vks {
namespace geometry {

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

void createSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, float radius, uint32_t sectors, uint32_t stacks);

} // namespace geometry
} // namespace vks

#endif // GEOMETRY_HPP
