#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 0) uniform GlobalBuffer {
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
    float time;
} ubo;

void main() {
    //Draw Depth
    float dist = length(ubo.cameraPosition - inWorldPos);
    float normalizedDist = clamp(dist / 100.0, 0.0, 1.0); // far = 100m
    outColor = vec4(vec3(normalizedDist), 1.0);

}
