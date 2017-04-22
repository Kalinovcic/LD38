#version 330 core

uniform sampler2D atlas;

in vec2 fragment_uv;

out vec4 fragment_color;

void main()
{
    fragment_color = texture(atlas, fragment_uv);
}
