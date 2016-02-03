#version 430
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_tex;

out vec2 texOut;

uniform mat4 Camera;
uniform mat4 Perspective;

void main()
{
	texOut = vertex_tex;
	gl_Position = Perspective * Camera * vec4(vertex_position, 1.0);
}

