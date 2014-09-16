#version 420

in layout (location = 0) vec3 vertex_position;

uniform mat4 proj_mat, view_mat, model_mat;

out vec3 position;

// skinny shader to only render simple polys

void main () {

	vec3 position_eye = vec3 (view_mat * model_mat * vec4 (vertex_position, 1.0));

	gl_Position = proj_mat * vec4 (position_eye, 1.0);
}