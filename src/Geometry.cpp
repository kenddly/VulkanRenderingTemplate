#include <vks/Geometry.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace vks {
namespace geometry {

void createSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, float radius, uint32_t sectors, uint32_t stacks) {
    vertices.clear();
    indices.clear();

    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;
    float s, t;

    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;
    float sectorAngle, stackAngle;

    for(uint32_t i = 0; i <= stacks; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for(uint32_t j = 0; j <= sectors; ++j) {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            s = (float)j / sectors;
            t = (float)i / stacks;

            vertices.push_back({{x, y, z}, {nx, ny, nz}, {s, t}});
        }
    }

    uint32_t k1, k2;
    for(uint32_t i = 0; i < stacks; ++i) {
        k1 = i * (sectors + 1);
        k2 = k1 + sectors + 1;

        for(uint32_t j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

} // namespace geometry
} // namespace vks
