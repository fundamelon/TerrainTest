#version 400
 
layout (location = 0) in vec3 position_vs_in;
layout (location = 1) in vec2 texcoord_vs_in;
layout (location = 2) in vec3 normal_vs_in;

uniform mat4 model_mat;

out vec3 worldpos_tc_in;
out vec2 texcoord_tc_in;
out vec3 normal_tc_in;
out vec3 normal_eye_tc_in;

 
void main () {

	worldpos_tc_in = vec3(model_mat * vec4(position_vs_in, 1.0)); // control points out == vertex points in
	normal_tc_in =  normal_vs_in;
	normal_eye_tc_in =  vec3(model_mat * vec4(normal_vs_in, 0.0));
	texcoord_tc_in = texcoord_vs_in;
}