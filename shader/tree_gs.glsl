#version 420

layout (points) in;

in VertexData {
    float scale;
	float type;
} VertexIn[];

layout (triangle_strip, max_vertices = 8) out;

layout (binding = 7) uniform sampler2D tex7;

uniform vec3 sun_direction;

uniform mat4 proj_mat, view_mat, model_mat;
uniform mat4 caster_proj_mat, caster_view_mat, caster_model_mat;
uniform float time;

int tree_dist_near = 0;
int tree_dist_far = 500;

out vec2 texcoord;
out float brightness_factor;
out float tree_dist;

float scale = VertexIn[0].scale;

vec4 shadow_coord;


float eval_shadow () {

	const float epsilon = 0.004;
	const int width = 2;
	const float div = 1.0/pow((width*2 + 1), 2);
	
	if(sun_direction.z < 0) return 0;

	float factor = 1;
	for(int i=-width; i <= width; i++) {
		for(int j = -width; j <= width; j++) {
			 if (texture(tex7, vec2(shadow_coord.x + i/2000.0, shadow_coord.y + j/2000.0)).z  <  shadow_coord.z-epsilon)
				factor -= div;
	  }
	}

	return factor;
}


void main() {

	// create a shadow map space texture coordinate
	shadow_coord = caster_proj_mat * caster_view_mat * caster_model_mat * vec4 (gl_in[0].gl_Position.xyz, 1.0);
	shadow_coord.xyz /= shadow_coord.w;
	shadow_coord.xyz += 1.0;
	shadow_coord.xyz *= 0.5;

	brightness_factor = eval_shadow();

	vec3 pos = vec3(model_mat * vec4(gl_in[0].gl_Position.xyz, 1.0));

	tree_dist = distance(vec3(0), pos);

	if(tree_dist < tree_dist_near) {
		// near action
	} else if(tree_dist > tree_dist_far) {
		// far action
	} else {

		float texcoord_x1, texcoord_x2, texcoord_y1, texcoord_y2;

		switch(VertexIn[0].type) {
			case 0:
				texcoord_x1 = 0.0;
				texcoord_x2 = 0.5;
				texcoord_y1 = 0.5;
				texcoord_y2 = 1.0;
				break;
			case 1:
				texcoord_x1 = 0.5;
				texcoord_x2 = 1.0;
				texcoord_y1 = 0.5;
				texcoord_y2 = 1.0;
				break;
			case 2:
				texcoord_x1 = 0.0;
				texcoord_x2 = 0.5;
				texcoord_y1 = 0.0;
				texcoord_y2 = 0.5;
				break;
			case 3:
				texcoord_x1 = 0.5;
				texcoord_x2 = 1.0;
				texcoord_y1 = 0.0;
				texcoord_y2 = 0.5;
				break;
			default:
				texcoord_x1 = 0.0;
				texcoord_x2 = 1.0;
				texcoord_y1 = 0.0;
				texcoord_y2 = 1.0;
				break;
		}

	//	vec3 pos = gl_in[0].gl_Position.xyz;

	//	vec3 right = vec3(1, 0, 0);
	//	vec3 forward = vec3(0, 1, 0);

	//	vec3 right = vec3(cos(3.1415 * (angle/2)), sin(3.1415 * (angle/2)), 0);
	//	vec3 forward = vec3(sin(3.1415 * (angle/2)), -cos(3.1415 * (angle/2)), 0);

	//	pos -= right * 0.5 * scale;
		pos.x -= 0.5 * scale;
		gl_Position =  proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_x1, texcoord_y1);
		EmitVertex();

		pos.z += 1.0 * scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_x1, texcoord_y2);
		EmitVertex();

		pos.z -= 1.0 * scale;
	//	pos += right * 1.0 * scale;
		pos.x += 1.0 * scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_x2, texcoord_y1);
		EmitVertex();
	
		pos.z += 1.0 * scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_x2, texcoord_y2);
		EmitVertex();

		EndPrimitive();

		pos.z -= 1.0 * scale;
	//	pos -= right * 0.5 * scale;
		pos.x -= 0.5 * scale;

	
	//	pos -= forward * 0.5 * scale;
		pos.y -= 0.5 * scale;
		gl_Position =  proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_x1, texcoord_y1);
		EmitVertex();

		pos.z += 1.0 * scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_x1, texcoord_y2);
		EmitVertex();

		pos.z -= 1.0 * scale;
	//	pos += forward * 1.0 * scale;
		pos.y += 1.0 * scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_x2, texcoord_y1);
		EmitVertex();
	
		pos.z += 1.0 * scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_x2, texcoord_y2);
		EmitVertex();

		EndPrimitive();
	}
}