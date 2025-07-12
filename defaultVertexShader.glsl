#version 410
layout (location=0) in vec3 position;

out vec4 misturaColor;

uniform mat4 mv_matrix;
uniform mat4 p_matrix;

void main(void){
	
	gl_Position = p_matrix * mv_matrix * vec4(position, 1.0);
	misturaColor = vec4(position, 1.0);
	

}