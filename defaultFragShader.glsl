#version 410 

uniform vec4 uColor;

in vec4 misturaColor;

out vec4 outColor;

void main(){

	outColor = misturaColor;
}