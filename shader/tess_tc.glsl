#version 400

layout (vertices = 3) out;

in vec3 worldpos_tc_in[];
in vec2 texcoord_tc_in[];
in vec3 normal_tc_in[];
in vec3 normal_eye_tc_in[];
 
uniform float tessLevelInner = 2.0;
uniform float tessLevelOuter = 2.0;
uniform int water;

out vec3 worldpos_te_in[];
out vec2 texcoord_te_in[];
out vec3 normal_te_in[];
out vec3 normal_eye_te_in[];
 
void main () {
	worldpos_te_in[gl_InvocationID] = worldpos_tc_in[gl_InvocationID];
	texcoord_te_in[gl_InvocationID] = texcoord_tc_in[gl_InvocationID];
	normal_te_in[gl_InvocationID] = normal_tc_in[gl_InvocationID];
	normal_eye_te_in[gl_InvocationID] = normal_eye_tc_in[gl_InvocationID];
 
	// Calculate the tessellation levels

	if(water == 0) {
		gl_TessLevelInner[0] = 1;
		gl_TessLevelOuter[0] = 1;
		gl_TessLevelOuter[1] = 1;
		gl_TessLevelOuter[2] = 1;
	} else if(water == 1) {
		gl_TessLevelInner[0] = tessLevelInner;
		gl_TessLevelOuter[0] = tessLevelOuter;
		gl_TessLevelOuter[1] = tessLevelOuter;
		gl_TessLevelOuter[2] = tessLevelOuter;
	}
}	