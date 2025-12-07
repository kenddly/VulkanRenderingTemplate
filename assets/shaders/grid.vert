#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

layout(set = 1, binding = 0) uniform GridMaterialUBO {
    vec4  baseColor;
    float spacing;
    int   dimension;
    float glowStrength;
    float glowPower;
    float nearFade;
    float farFade;
} mat;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec3 vWorldPos;
layout(location = 2) out vec3 vCamPos;

void main() {
    const float SPACING = mat.spacing;
    const int GRID_DIM = int(mat.dimension);
    const float GRID_SIZE = SPACING * float(GRID_DIM);
    
    float t = float(gl_VertexIndex); 

    int S = 2 * GRID_DIM + 1;
    int axis = gl_InstanceIndex / (S * S);
    int indexInAxis = gl_InstanceIndex % (S * S);

    int row = indexInAxis / S;
    int col = indexInAxis % S;

    float u = (float(row) - float(GRID_DIM)) * SPACING;
    float v = (float(col) - float(GRID_DIM)) * SPACING;

    float lineStart = -GRID_SIZE;
    float lineEnd   =  GRID_SIZE;
    float linePos   = mix(lineStart, lineEnd, t);

    vec3 localPos;
    if (axis == 0)      localPos = vec3(linePos, u, v);
    else if (axis == 1) localPos = vec3(u, linePos, v);
    else                localPos = vec3(u, v, linePos);

    vec3 camPos = ubo.camPos;
    vec3 snap = floor(camPos / SPACING) * SPACING;
    vec3 worldPos = localPos + snap;

    vec4 clip = ubo.proj * ubo.view * vec4(worldPos, 1.0);
    gl_Position = clip;

    vColor = mat.baseColor;
    vWorldPos = worldPos;
    vCamPos = ubo.camPos;
}
