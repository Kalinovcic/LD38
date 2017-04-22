#version 330 core

uniform mat4 transform;

layout(location = 0) in vec2 vertex_position;
layout(location = 1) in vec2 vertex_uv;

out vec2 fragment_uv;

void main()
{
    gl_Position = transform * vec4(vertex_position, 0, 1);
    fragment_uv = vertex_uv;
}
