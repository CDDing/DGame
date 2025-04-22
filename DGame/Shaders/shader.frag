#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : require
struct Material {
    int baseColorIndex;
    int normalMapIndex;
    int metallicRoughnessIndex;
    int emissiveIndex;

    vec4 baseColorFactor;

    float metallicFactor;
    float roughnessFactor;
    vec2 _padding1;

    vec3 emissiveFactor;
    float _padding2;
};

layout(set = 1, binding = 0) readonly buffer MaterialBuffer {
    Material materials[];
};
layout(set = 1, binding = 1) uniform sampler2D textures[];

layout(push_constant) uniform PushConstant {
    mat4 modelMatrix;
    uint64_t vertexAddress;
    int materialIndex;
    int pad;
} pushConst;


layout(set = 0, binding = 0) uniform GlobalBuffer {
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
    float time;
} ubo;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    Material mat = materials[pushConst.materialIndex];

    vec4 baseColor = mat.baseColorFactor;

    if(mat.baseColorIndex >=0){
        baseColor = texture(nonuniformEXT(textures[mat.baseColorIndex]),inUV);
    }
    outColor = baseColor;
}
