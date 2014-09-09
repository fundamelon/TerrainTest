#version 400

in vec3 vertex_position;
uniform mat4 proj_mat, view_mat, model_mat;

void main () {
	gl_Position = proj_mat * view_mat * model_mat * vec4 (vertex_position, 1.0);
}