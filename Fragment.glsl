#version 400

out vec3 fragment_color;

in vec2 texOut;
in vec3 normal;
in vec3 light;
uniform sampler2D texSampler;

uniform vec3 ambientLight;
uniform vec3 diffuseLight;
uniform vec3 specularLight;

void main () 
{
	vec4 mySample = texture(texSampler, vec2(texOut.s, 1- texOut.t));
	float diffuse = dot(normal, light);
	fragment_color = mySample.rgb + ambientLight + diffuseLight*diffuse;
}