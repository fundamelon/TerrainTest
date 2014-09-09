#version 420

in layout (location = 0) vec3 vertex_position;
in layout (location = 1) vec3 vertex_normal;
in layout (location = 2) vec2 vertex_texcoord;

uniform mat4 proj_mat, view_mat, model_mat;
uniform mat4 caster_proj_mat, caster_view_mat, caster_model_mat;
uniform float time;
uniform int type;


out vec3 position, normal, position_eye, normal_eye;
out vec2 texcoord;
out vec4 shadow_coord;


void main () {

	// create a shadow map space texture coordinate
	shadow_coord = caster_proj_mat * caster_view_mat * caster_model_mat * vec4 (vertex_position, 1.0);
	shadow_coord.xyz /= shadow_coord.w;
	shadow_coord.xyz += 1.0;
	shadow_coord.xyz *= 0.5;

	position = vertex_position;
	normal = vertex_normal;
	texcoord = vertex_texcoord;

	position_eye = vec3 (view_mat * model_mat * vec4 (vertex_position, 1.0));
	normal_eye = vec3 (view_mat * model_mat * vec4 (vertex_normal, 0.0));

	gl_Position = proj_mat * vec4 (position_eye, 1.0);
}