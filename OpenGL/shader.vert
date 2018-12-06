#version 450

in vec3 position;
in vec3 colorParticule;
out vec3 colorIN;

void main()
{
    gl_Position = vec4(position, 1.0);
	colorIN = colorParticule;
}