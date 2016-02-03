#version 430

out vec3 fragment_color;

in vec2 texOut;

uniform sampler2D texSampler;

void main () 
{
	vec4 mySample = texture(texSampler, vec2(texOut.s, 1- texOut.t));
	fragment_color = mySample.rgb;
}