#version 420

in vec3 vertex_position;
uniform mat4 proj_mat, view_mat;
out vec3 position_world;

void main () {
	position_world = vertex_position;
	gl_Position = proj_mat * view_mat * vec4 (vertex_position, 1.0);
}