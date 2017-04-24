#version 330 core

uniform sampler2D atlas;
uniform vec4 color_multiplier;

in vec2 fragment_uv;

out vec4 fragment_color;

void main()
{
    fragment_color = texture(atlas, fragment_uv) * color_multiplier;
}
