#version 420

layout (points) in;

in VertexData {
	vec3 pos;
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
int tree_dist_far = 400;

out vec3 forward;
out vec2 texcoord;
out float brightness_factor;
out float tree_dist;

float scale = VertexIn[0].scale;
float half_scale = scale/2.0;

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

const vec3 texcoord_set = vec3(0.0, 0.5, 1.0);

void main() {

	vec3 pos = vec3(model_mat * vec4(VertexIn[0].pos, 1.0));

	tree_dist = length(pos);

	if(tree_dist < tree_dist_near) {
		// near action
	} else if(tree_dist > tree_dist_far) {
		// far action
	} else {

		// create a shadow map space texture coordinate
		shadow_coord = caster_proj_mat * caster_view_mat * caster_model_mat * vec4 (VertexIn[0].pos, 1.0);
		shadow_coord.xyz /= shadow_coord.w;
		shadow_coord.xyz += 1.0;
		shadow_coord.xyz *= 0.5;

		brightness_factor = eval_shadow();

		vec4 texcoord_vals;

		switch(VertexIn[0].type) {
			case 0:
				// 0.0, 0.5, 0.5, 1.0
				texcoord_vals = texcoord_set.xyyz;
				break;
			case 1:
				// 0.5, 1.0, 0.5, 1.0
				texcoord_vals = texcoord_set.yzyz;
				break;
			case 2:
				// 0.0, 0.5, 0.0, 0.5
				texcoord_vals = texcoord_set.xyxy;
				break;
			case 3:
				// 0.5, 1.0, 0.0, 0.5
				texcoord_vals = texcoord_set.yzxy;
				break;
			default:
				// 0.0, 1.0, 0.0, 1.0
				texcoord_vals = texcoord_set.xzxz;
				break;
		}
		
		vec3 pos_world = pos;
		//pos = VertexIn[0].pos;

		forward = -normalize(pos_world);
		vec3 global_up = vec3(0.0, 0.0, 1.0);
		vec3 right = normalize(cross(forward, global_up));
		vec3 up = -normalize(cross(forward, right));

		pos.z += half_scale;
		pos -= up * half_scale;
		pos -= right * half_scale;
		gl_Position =  proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.x, texcoord_vals.z);
		EmitVertex();

		pos += up * scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.x, texcoord_vals.w);
		EmitVertex();

		pos -= up * scale;
		pos += right * scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.y, texcoord_vals.z);
		EmitVertex();
		
		pos += up * scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.y, texcoord_vals.w);
		EmitVertex();

		EndPrimitive();

		/*

		pos.x -= half_scale;
		gl_Position =  proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.x, texcoord_vals.z);
		EmitVertex();

		pos.z += scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.x, texcoord_vals.w);
		EmitVertex();

		pos.z -= scale;
		pos.x += scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.y, texcoord_vals.z);
		EmitVertex();
	
		pos.z += scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.y, texcoord_vals.w);
		EmitVertex();

		EndPrimitive();

		pos.z -= scale;
		pos.x -= half_scale;

		pos.y -= half_scale;
		gl_Position =  proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.x, texcoord_vals.z);
		EmitVertex();

		pos.z += scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.x, texcoord_vals.w);
		EmitVertex();

		pos.z -= scale;
		pos.y += scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.y, texcoord_vals.z);
		EmitVertex();
	
		pos.z += scale;
		gl_Position = proj_mat * view_mat * vec4(pos, 1.0);
		texcoord = vec2(texcoord_vals.y, texcoord_vals.w);
		EmitVertex();

		EndPrimitive();

		*/
	}
}