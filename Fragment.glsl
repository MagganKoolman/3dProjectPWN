#version 400

out vec3 fragment_color;

in vec2 texOut;
in vec3 normal;
in vec3 position;

uniform sampler2D texSampler;

uniform vec3 cameraPos;
uniform vec3 ambientLight;
uniform vec3 diffuseLight;
uniform vec3 specularLight;

void main () 
{
	vec3 light = vec3(0,5,10);
	vec3 lv = normalize(position - light);
	vec3 r = reflect(lv, normal);
	vec3 vc = normalize(cameraPos - position);

	vec3 specular = diffuseLight * pow(max(dot(vc, r),0),20);
	vec4 mySample = texture(texSampler, vec2(texOut.s, 1- texOut.t));
	vec3 diffuse = diffuseLight* max(dot(normal, -lv),0);
	fragment_color = 0.5*(mySample.rgb + ambientLight + diffuse) + 2*specular;
}