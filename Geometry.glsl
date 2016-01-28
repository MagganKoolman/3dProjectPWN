#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 6) out;

out vec3 normal;
out vec3 vertexPos;

in vec2 texOut[];
out vec2 texOut2;

in vec4 position[];

uniform mat4 MV;
uniform mat4 P;
uniform mat4 normMatrix;
 
void main()
{	
	vec3 p1 = position[1].xyz - position[0].xyz;
	vec3 p2 = position[2].xyz - position[0].xyz;
	vec3 norm = normalize(cross(p1, p2));
	vec4 pos[3];
	
	normal = normalize(vec3(normMatrix * vec4(norm,0.0)));

	for (int i = 0; i < 3; i++)
	{
		pos[i] = MV * position[i];
	}
	
	for(int i=0; i<3; i++)
	{
		vertexPos = vec3(pos[i]);
		gl_Position = P * pos[i];
		texOut2 = texOut[i];
		EmitVertex();
	}
	EndPrimitive();

	for(int i=0; i<3; i++)
	{
		vertexPos = vec3(pos[i] + vec4(normal, 0.0));
		gl_Position = P * ((pos[i]) + vec4(normal, 0.0));		
		texOut2 = texOut[i];
		EmitVertex();
	}
	EndPrimitive();
}  