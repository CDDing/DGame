#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 0) out vec4 outDepth;
layout(set = 0, binding = 0) uniform GlobalBuffer {
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
    float time;
} ubo;

void main() {
    //Draw Depth
    vec4 viewPos = ubo.view * vec4(inWorldPos, 1.0);
    float depthValue = viewPos.z;
    outDepth = vec4(depthValue, depthValue, depthValue, 1);
}
