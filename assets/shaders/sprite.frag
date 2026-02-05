#version 450

// Inputs from Vertex Shader
layout(location = 0) in vec2 fragUV;

// Outputs
layout(location = 0) out vec4 outColor;

// Sprite Texture (Set 1)
layout(set = 1, binding = 1) uniform sampler2D spriteTexture;

void main()
{
    vec4 color = texture(spriteTexture, fragUV);

    // Optional: early discard for cleaner edges
    if (color.a < 0.01)
        discard;

    outColor = color;
}
