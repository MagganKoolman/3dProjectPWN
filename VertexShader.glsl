#version 400
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_tex;
layout(location = 2) in vec3 vertex_normal;

out vec2 texOut;
out vec3 normal;
out vec3 position;

uniform mat4 Model;
uniform mat4 Camera;
uniform mat4 Perspective;

void main()
{
	vec3 temp = vertex_position;
	position = temp;
	normal = vertex_normal;
	texOut = vertex_tex;
	gl_Position = Perspective * Camera * Model * vec4(temp, 1.0);
}

