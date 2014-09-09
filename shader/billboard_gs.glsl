#version 420

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform mat4 proj_mat, view_mat, model_mat;

mat4 model_mat_mask = mat4(
	1.0, 0.0, 0.0, 0.0, 
	0.0, 1.0, 0.0, 0.0, 
	0.0, 0.0, 0.0, 0.0, 
	0.0, 0.0, 0.0, 1.0
);

out vec2 texcoord;

void main() {

	vec3 pos = gl_in[0].gl_Position.xyz;
    vec3 pos_world = vec3(model_mat * vec4(gl_in[0].gl_Position.xyz, 1.0));
//	vec3 pos_eye = vec3(proj_mat * view_mat * vec4(pos_world, 1.0));

    vec3 forward = -normalize(pos_world);
    vec3 global_up = vec3(0.0, 0.0, 1.0);
    vec3 right = normalize(cross(forward, global_up));
	vec3 up = -normalize(cross(forward, right));

	pos.z += 0.5;
	pos -= up * 0.5;
	pos -= right * 0.5;
    gl_Position =  proj_mat * view_mat * model_mat * vec4(pos, 1.0);
    texcoord = vec2(0.0, 0.0);
    EmitVertex();

 //   pos.z += 1.0;
	pos += up;
    gl_Position = proj_mat * view_mat * model_mat * vec4(pos, 1.0);
    texcoord = vec2(0.0, 1.0);
    EmitVertex();

//	pos.z -= 1.0;
	pos -= up;
	pos += right;
    gl_Position = proj_mat * view_mat * model_mat * vec4(pos, 1.0);
    texcoord = vec2(1.0, 0.0);
    EmitVertex();
	
 //   pos.z += 1.0;
	pos += up;
    gl_Position = proj_mat * view_mat * model_mat * vec4(pos, 1.0);
    texcoord = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}