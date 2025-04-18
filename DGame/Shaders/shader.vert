#version 460

void main() {
    // Predefined triangle in clip space
    vec2 positions[3] = vec2[](
        vec2( 0.0, -0.5),  // Bottom
        vec2( 0.5,  0.5),  // Right
        vec2(-0.5,  0.5)   // Left
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
