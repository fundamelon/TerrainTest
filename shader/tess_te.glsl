#version 400
 
layout (triangles, equal_spacing, ccw) in;

in vec3 worldpos_te_in[];
in vec2 texcoord_te_in[];
in vec3 normal_te_in[];
in vec3 normal_eye_te_in[];

// could use a displacement map here
 
uniform mat4 projection_mat, view_mat, model_mat;

out vec3 position, normal, position_eye, normal_eye;
out vec2 texcoord;

 
void main () {

	vec3 p0 = gl_TessCoord.x * worldpos_te_in[0]; // x is one corner
	vec3 p1 = gl_TessCoord.y * worldpos_te_in[1]; // y is the 2nd corner
	vec3 p2 = gl_TessCoord.z * worldpos_te_in[2]; // z is the 3rd corner (ignore when using quads)

	position = normalize (p0 + p1 + p2);
	
	vec3 n0 = gl_TessCoord.x * normal_te_in[0]; // x is one corner
	vec3 n1 = gl_TessCoord.y * normal_te_in[1]; // y is the 2nd corner
	vec3 n2 = gl_TessCoord.z * normal_te_in[2]; // z is the 3rd corner (ignore when using quads)

	normal = normalize (n0 + n1 + n2);
//	normal = normal_te_in[0];
//	normal = vec3(0, 0, 1);

	vec3 ne0 = gl_TessCoord.x * normal_eye_te_in[0]; // x is one corner
	vec3 ne1 = gl_TessCoord.y * normal_eye_te_in[1]; // y is the 2nd corner
	vec3 ne2 = gl_TessCoord.z * normal_eye_te_in[2]; // z is the 3rd corner (ignore when using quads)

	normal = normalize (n0 + n1 + n2);

	position_eye = vec3 (view_mat * vec4 (position, 1.0));
	normal_eye = vec3 (view_mat * vec4 (normal, 0.0));

	gl_Position = projection_mat * vec4 (position_eye, 1.0);
}