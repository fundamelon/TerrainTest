#version 420

in layout (location = 0) vec3 obj_position;
in layout (location = 1) float obj_scale;
in layout (location = 2) float obj_type;

out VertexData {
	float scale;
	float type;
} VertexOut;


void main () {

	VertexOut.scale = obj_scale;
	VertexOut.type = obj_type;
	gl_Position = vec4 (obj_position, 1.0);
}