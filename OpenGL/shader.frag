#version 450

out vec4 color;
in vec3 colorIN;

void main()
{
    color = vec4(colorIN, 1);
}