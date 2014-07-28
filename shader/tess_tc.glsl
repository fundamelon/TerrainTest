#version 400

layout (vertices = 3) out;

in vec3 position_tc_in[];
in vec3 normal_tc_in[];
in vec2 texcoord_tc_in[];
in vec3 worldpos_tc_in[];
 
uniform float tessLevelInner = 2.0;
uniform float tessLevelOuter = 2.0;
uniform int type;

out vec3 position_te_in[];
out vec3 normal_te_in[];
out vec2 texcoord_te_in[];


float get_tess_level() {

	return clamp(20 - distance(vec3(0.0f), worldpos_tc_in[gl_InvocationID])/4, 1.0, 13.0);
}

 
void main () {
	position_te_in[gl_InvocationID] = position_tc_in[gl_InvocationID];
	texcoord_te_in[gl_InvocationID] = texcoord_tc_in[gl_InvocationID];
	normal_te_in[gl_InvocationID] = normal_tc_in[gl_InvocationID];
 
	// Calculate the tessellation levels

	float tess_level = get_tess_level();

	if(type == 1) {
		gl_TessLevelInner[0] = tess_level;
		gl_TessLevelOuter[0] = tess_level;
		gl_TessLevelOuter[1] = tess_level;
		gl_TessLevelOuter[2] = tess_level;
	} else {
		gl_TessLevelInner[0] = 1;
		gl_TessLevelOuter[0] = 1;
		gl_TessLevelOuter[1] = 1;
		gl_TessLevelOuter[2] = 1;
	} 
}	