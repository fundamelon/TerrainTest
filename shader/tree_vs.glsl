#version 420

in layout (location = 0) vec3 obj_position;
in layout (location = 1) float obj_scale;
in layout (location = 2) float obj_type;

uniform mat4 proj_mat, view_mat, model_mat;

out VertexData {
	vec3 pos; 
	float scale;
	float type;
} VertexOut;


void main () {

	VertexOut.pos = obj_position;
	VertexOut.scale = obj_scale;
	VertexOut.type = obj_type;
	gl_Position = proj_mat * view_mat * model_mat * vec4 (obj_position, 1.0);
}