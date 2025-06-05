#version 450

layout(push_constant) uniform PushConstants {
    mat4 view;
    mat4 proj;
} ps;

layout(location = 0) out vec3 outFragPosition;

const vec3 g_VertexPositions[36] = vec3[](
    // back face
    vec3(-1.0, -1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),

    // front face
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),

    // left face
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),

    // right face
    vec3(1.0,  1.0,  -1.0),
    vec3(1.0,  1.0,   1.0),
    vec3(1.0, -1.0,   1.0),
    vec3(1.0, -1.0,   1.0),
    vec3(1.0, -1.0,  -1.0),
    vec3(1.0,  1.0,  -1.0),

    // bottom face
    vec3(-1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3(-1.0, -1.0, -1.0),

    // top face
    vec3(-1.0, 1.0,  1.0),
    vec3( 1.0, 1.0,  1.0),
    vec3( 1.0, 1.0, -1.0),
    vec3( 1.0, 1.0, -1.0),
    vec3(-1.0, 1.0, -1.0),
    vec3(-1.0, 1.0,  1.0)
);


void main()
{
    vec3 position = g_VertexPositions[gl_VertexIndex];
    outFragPosition = position;
    gl_Position = ps.proj * ps.view * vec4(position.xyz, 1.0);
}