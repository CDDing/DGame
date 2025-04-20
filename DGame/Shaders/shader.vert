#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(push_constant) uniform PushConstant {
    mat4 modelMatrix;
    uint64_t vertexAddress;
} pushConst;

struct Vertex{
vec3 position;
float pad;
vec3 normal;
float pad2;
vec2 texcoord;
vec2 pad3;
};

layout(buffer_reference, scalar) readonly buffer VertexBuffer{
Vertex vertices[];
};

void main() {
    // Predefined triangle in clip space
    VertexBuffer vb = VertexBuffer(pushConst.vertexAddress);
    Vertex v = vb.vertices[gl_VertexIndex];

    gl_Position = pushConst.modelMatrix * vec4(v.position,1.0);
}
