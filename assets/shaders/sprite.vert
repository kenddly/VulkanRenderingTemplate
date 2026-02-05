#version 450

// Vertex Inputs
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

// Outputs to Fragment Shader
layout(location = 0) out vec2 fragUV;

// Global Camera UBO (Set 0)
layout(set = 0, binding = 0) uniform CameraUBO
{
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} camera;

// Push Constants
layout(push_constant) uniform Push
{
    mat4 model;
} push;

void main()
{
    fragUV = inUV;

    gl_Position =
        camera.proj *
        camera.view *
        push.model *
        vec4(inPosition, 1.0);
}
