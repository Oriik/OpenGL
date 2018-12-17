#version 450

in vec3 position;

uniform mat4 trans;

void main()
{
    gl_Position = trans *  vec4(position, 1.0);
	
}