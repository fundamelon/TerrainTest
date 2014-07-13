#version 420

in layout (location = 0) vec3 vertex_position;
in layout (location = 1) vec3 vertex_normal;
uniform mat4 projection_mat, view_mat, model_mat;
uniform mat4 caster_proj, caster_view, caster_model;
out vec3 normal, position_eye, normal_eye;

out vec4 shadow_coord;

void main () {
	normal = vertex_normal;
	position_eye = vec3 (view_mat * model_mat * vec4 (vertex_position, 1.0));
	normal_eye = vec3 (view_mat * model_mat * vec4 (vertex_normal, 0.0));
	gl_Position = projection_mat * vec4 (position_eye, 1.0);

	// create a shadow map texture coordinate by backwards-ising the position.
	shadow_coord = caster_proj * caster_view * caster_model * vec4 (vertex_position, 1.0);
	shadow_coord.xyz /= shadow_coord.w;
	shadow_coord.xyz += 1.0;
	shadow_coord.xyz *= 0.5;
}