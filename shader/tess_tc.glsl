#version 400

layout (vertices = 3) out;

in vec3 position_tc_in[];
in vec3 normal_tc_in[];
in vec2 texcoord_tc_in[];
in vec3 worldpos_tc_in[];
 
uniform float tessLevelInner = 2.0;
uniform float tessLevelOuter = 2.0;
uniform int water;

out vec3 position_te_in[];
out vec3 normal_te_in[];
out vec2 texcoord_te_in[];


float get_tess_level() {

	return clamp(5 - distance(vec3(0.0f), worldpos_tc_in[gl_InvocationID])/8, 1.0, 3.0);
}

 
void main () {
	position_te_in[gl_InvocationID] = position_tc_in[gl_InvocationID];
	texcoord_te_in[gl_InvocationID] = texcoord_tc_in[gl_InvocationID];
	normal_te_in[gl_InvocationID] = normal_tc_in[gl_InvocationID];
 
	// Calculate the tessellation levels

	float tess_level = get_tess_level();

	if(water == 0) {
		gl_TessLevelInner[0] = 1;
		gl_TessLevelOuter[0] = 1;
		gl_TessLevelOuter[1] = 1;
		gl_TessLevelOuter[2] = 1;
	} else if(water == 1) {
		gl_TessLevelInner[0] = tess_level;
		gl_TessLevelOuter[0] = tess_level;
		gl_TessLevelOuter[1] = tess_level;
		gl_TessLevelOuter[2] = tess_level;
	}
}	