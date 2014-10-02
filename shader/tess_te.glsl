#version 400
 
layout (triangles, equal_spacing, ccw) in;

layout (binding = 1) uniform sampler2D disp_tex;

in vec3 position_te_in[];
in vec3 normal_te_in[];
in vec2 texcoord_te_in[];

// could use a displacement map here
 
uniform mat4 proj_mat, view_mat, model_mat;
uniform mat4 caster_proj, caster_view, caster_model;
uniform float time;
uniform int type;

out vec3 position, normal, position_eye, normal_eye;
out vec2 texcoord;
out vec4 shadow_coord;

mat4 view_mat_mul = mat4(
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
);


void main () {

	vec3 p0 = gl_TessCoord.x * position_te_in[0];
	vec3 p1 = gl_TessCoord.y * position_te_in[1];
	vec3 p2 = gl_TessCoord.z * position_te_in[2];

	position = p0 + p1 + p2;
	
	vec3 n0 = gl_TessCoord.x * normal_te_in[0];
	vec3 n1 = gl_TessCoord.y * normal_te_in[1];
	vec3 n2 = gl_TessCoord.z * normal_te_in[2];

	normal = normalize (n0 + n1 + n2);
	
	vec2 tc0 = gl_TessCoord.x * texcoord_te_in[0];
	vec2 tc1 = gl_TessCoord.y * texcoord_te_in[1];
	vec2 tc2 = gl_TessCoord.z * texcoord_te_in[2];

	texcoord = tc0 + tc1 + tc2;
	
	if(type==1) {
	//	normal.x = mix(normal.x, texture(disp_tex, texcoord).g * 2.0, clamp(distance(vec3(0.0f), position)/10, 0.0, 1.0));
	//	normal.x += snoise(vec3(position.x, position.y, time));
	//	position_vs_in.z = (texcoord_vs_in.s + texcoord_vs_in.t) * 2.0;
	}

	// create a shadow map texture coordinate
	shadow_coord = caster_proj * caster_view * caster_model * vec4 (position, 1.0);
	shadow_coord.xyz /= shadow_coord.w;
	shadow_coord.xyz += 1.0;
	shadow_coord.xyz *= 0.5;

	if(type == 2) {
		position_eye = vec3 (view_mat_mul * view_mat * model_mat * vec4 (position, 1.0));
	} else {
		position_eye = vec3 (view_mat * model_mat * vec4 (position, 1.0));
	}

	normal_eye = vec3 (view_mat * model_mat * vec4 (normal, 0.0));

	gl_Position = proj_mat * vec4 (position_eye, 1.0);
}