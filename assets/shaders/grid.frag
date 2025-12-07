#version 450

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec3 vWorldPos;
layout(location = 2) in vec3 vCamPos;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform GridMaterialUBO {
    vec4  baseColor;
    float spacing;
    int   dimension;
    float glowStrength;
    float glowPower;
    float nearFade;
    float farFade;
    float time;
} mat;

void main() {
    float dist = length(vWorldPos - vCamPos);
    float fadeStart = float(mat.dimension) * 0.25;
    float fadeEnd   = float(mat.dimension) * 1.25;

    float k = clamp((dist - fadeStart) / (fadeEnd - fadeStart), 0.0, 1.0);
    float alpha = pow(1.0 - k, 2.5);

    // ---- BASIC GLOW ----
    float baseGlow = pow(alpha, mat.glowPower) * mat.glowStrength;

    // ---- LINE DIRECTION DETECTION ----
    vec3 a = abs(dFdx(vWorldPos));
    vec3 b = abs(dFdy(vWorldPos));
    vec3 deriv = max(a, b);

    bool isX = deriv.x > deriv.y && deriv.x > deriv.z;
    bool isY = deriv.y > deriv.x && deriv.y > deriv.z;
    bool isZ = deriv.z > deriv.x && deriv.z > deriv.y;

    // ---- GET LINE PARAMETER T ----
    float t = (isX ? vWorldPos.x :
              (isY ? vWorldPos.y :
                     vWorldPos.z));

    // ---- TRAVELING PARTICLE GLOW ----
    float pulseWidth = 0.45;
    float speed = 1;
    float freq = 2.0;   // particles per line

    float phase = t * freq + mat.time * speed;
    float particlePulse = exp(-pow(fract(phase) - 0.5, 2.0) / pulseWidth);

    // ---- FINAL COLOR ----
    float glow = baseGlow + particlePulse * 1.5; // boost particles
    vec3 glowColor = vColor.rgb * glow;

    outColor = vec4(glowColor, alpha);
}
