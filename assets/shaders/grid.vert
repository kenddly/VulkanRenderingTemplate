#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

struct Sphere {
    vec3 center;
    float radius;
    float mass;
};

const int MAX_SPHERES = 32;

layout(set = 1, binding = 0) uniform GridMaterialUBO {
    vec4  baseColor;

    float spacing;
    int   dimension;

    float glowStrength;
    float glowPower;

    float nearFade;
    float farFade;

    float time;

    Sphere spheres[MAX_SPHERES];
    int sphereCount;
    float softening;
    float curvatureK;

    // ---- NEW FIELDS ----
    int lineVertexCount;   // number of vertices per line (must be >= 2)
    int integratorSteps;   // steps the integrator will take per-vertex
} mat;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec3 vWorldPos;
layout(location = 2) out vec3 vCamPos;

// safe length to avoid divide-by-zero
float safeLength(vec3 v) { return max(length(v), 1e-6); }

// compute acceleration-like vector at point p from all spheres
vec3 AccelAtPoint(vec3 p)
{
    vec3 accel = vec3(0.0);
    for (int i = 0; i < mat.sphereCount; ++i) {
        Sphere s = mat.spheres[i];
        vec3 r = p - s.center;
        float d = safeLength(r);
        float safeD = max(d, mat.softening);

        // soften inside radius a bit
        float effD = max(safeD, s.radius * 0.5);

        // inverse-square style magnitude (not exact GR but visually good)
        float a = mat.curvatureK * s.mass / (effD * effD);

        // direction toward center (negative r)
        accel += (-normalize(r)) * a;
    }
    return accel;
}

// integrate a curved path starting at pStart in initial direction dir0
// for a distance totalDistance using 'steps' substeps.
// returns endpoint position
vec3 IntegrateCurvedPath(vec3 pStart, vec3 dir0, float totalDistance, int steps)
{
    if (totalDistance <= 0.0 || steps <= 0) return pStart;

    vec3 p = pStart;
    vec3 dir = normalize(dir0);

    float stepLen = totalDistance / float(steps);
    // dt scales the impact of accel on direction; tune if needed.
    const float dt = 1.0;

    for (int i = 0; i < steps; ++i) {
        vec3 a = AccelAtPoint(p);

        // update direction - simple semi-stable Euler-ish update
        dir += a * dt * 0.5; // small factor to stabilize
        dir = normalize(dir);

        p += dir * stepLen;
    }

    return p;
}

void main()
{
    const float SPACING = mat.spacing;
    const int GRID_DIM = mat.dimension;
    const float GRID_SIZE = SPACING * float(GRID_DIM);

    // Use lineVertexCount passed from CPU. Guard against invalid value.
    int lineVerts = max(2, mat.lineVertexCount);

    // compute param t in [0,1] using (lineVerts - 1) to include both endpoints
    float t = float(gl_VertexIndex) / float(max(lineVerts - 1, 1));

    int S = 2 * GRID_DIM + 1;
    int axis = gl_InstanceIndex / (S * S);
    int idx  = gl_InstanceIndex % (S * S);

    int row = idx / S;
    int col = idx % S;

    float u = (float(row) - float(GRID_DIM)) * SPACING;
    float v = (float(col) - float(GRID_DIM)) * SPACING;

    float lineStart = -GRID_SIZE;
    float lineEnd   =  GRID_SIZE;
    float totalLineLength = lineEnd - lineStart; // = 2*GRID_SIZE

    // start point of the Euclidean line
    vec3 localStart;
    vec3 basisDir;
    if (axis == 0) {
        localStart = vec3(lineStart, u, v);
        basisDir = vec3(1.0, 0.0, 0.0);
    } else if (axis == 1) {
        localStart = vec3(u, lineStart, v);
        basisDir = vec3(0.0, 1.0, 0.0);
    } else {
        localStart = vec3(u, v, lineStart);
        basisDir = vec3(0.0, 0.0, 1.0);
    }

    // snap grid to camera origin to reduce precision issues
    vec3 snap = floor(ubo.camPos / SPACING) * SPACING;
    vec3 p0 = localStart + snap;

    // compute travel distance along the line for this vertex
    float travelDist = t * totalLineLength;

    // clamp integrator steps to reasonable range inside shader
    int steps = mat.integratorSteps;
    if (steps < 1) steps = 1;
    const int MAX_STEPS = 64;
    if (steps > MAX_STEPS) steps = MAX_STEPS;

    // integrate along the curved path
    vec3 worldPos = IntegrateCurvedPath(p0, basisDir, travelDist, steps);

    // project
    gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.0);

    // pass-throughs
    vColor = mat.baseColor;
    vWorldPos = worldPos;
    vCamPos = ubo.camPos;
}