#version 430

out vec3 fragment_color;

in vec3 texOut;

void main () 
{
	fragment_color = texOut;
}