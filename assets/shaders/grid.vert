#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_debug_printf : enable

layout(binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {
    vec4 cameraPos; // .xyz = world-space camera position
    vec4 color;
    vec2 settings; // SPACING, GRID_DIM, unused
} push;

layout(location = 0) out vec4 vColor; // vary to fragment shader


void main() {
    const float SPACING = push.settings.x;
    const int GRID_DIM = int(push.settings.y);
    const float GRID_SIZE = float(GRID_DIM) * SPACING;
    
    // 0 or 1
    float t = float(gl_VertexIndex); // 0.0 or 1.0

    // number of lines per axis = (2*GRID_DIM + 1)^2
    int S = 2 * GRID_DIM + 1;
    int axis = gl_InstanceIndex / (S * S);
    int indexInAxis = gl_InstanceIndex % (S * S);

    int row = indexInAxis / S;
    int col = indexInAxis % S;

    float u = (float(row) - float(GRID_DIM)) * SPACING;
    float v = (float(col) - float(GRID_DIM)) * SPACING;

    // line range across whole grid box
    float lineStart = -GRID_SIZE;
    float lineEnd   =  GRID_SIZE;
    float linePos   = mix(lineStart, lineEnd, t);

    vec3 localPos;
    if (axis == 0) {
        // vary X
        localPos = vec3(linePos, u, v);
    } else if (axis == 1) {
        // vary Y
        localPos = vec3(u, linePos, v);
    } else {
        // vary Z
        localPos = vec3(u, v, linePos);
    }

    // Snap-to-camera to make grid "infinite" (optional):
    // Snap offset keeps grid aligned to spacing grid around camera
//    vec3 camPos = push.cameraPos.xyz;
    vec3 camPos = vec3(1, 2, 3);
    vec3 snap = floor(camPos / SPACING) * SPACING;
    vec3 worldPos = localPos + snap;

    // Transform
    vec4 clip = ubo.proj * ubo.view * vec4(worldPos, 1.0);
    gl_Position = clip;

    // Fade based on world-space distance to camera (more correct)
    float dist = length(worldPos - camPos);

    // smooth fade: 1.0 near, 0.0 at farLimit
    float farLimit = GRID_SIZE * 1.2; // tweak
    float alpha = clamp(1.0 - (dist / farLimit), 0.0, 1.0);
    // soften edge
    alpha = smoothstep(0.0, 1.0, alpha);

    // send color (apply alpha)
    vColor = vec4(push.color.rgb, push.color.a * alpha);
    
    debugPrintfEXT("Instance %d, Axis %d, Row %d, Col %d, Pos (%.1f, %.1f, %.1f), Dist %.1f, Alpha %.2f\n",
        gl_InstanceIndex, axis, row, col,
        worldPos.x, worldPos.y, worldPos.z,
        dist,
        alpha
    );
}
