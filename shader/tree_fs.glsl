#version 420

in vec3 forward;
in vec2 texcoord;
in float tree_dist;
in float brightness_factor;

layout (binding = 0) uniform lowp sampler2D tex0;
layout (binding = 1) uniform sampler2D tex1;

uniform vec3 sun_direction;
uniform lowp float sun_dot;
uniform mat4 view_mat, model_mat;

int tree_dist_near = 0;
int tree_dist_far = 400;

out vec4 frag_color;


void main() {

	frag_color = texture(tex0, texcoord.st);

	if(frag_color.a <= 0.5) discard;

	float global_brightness = 1.0;//sun_dot * 0.4 + 0.6;

	vec3 normal = normalize(texture(tex1, texcoord).rgb);// + forward * 0.2;

	vec3 normal_eye = vec3 (view_mat * model_mat * vec4 (normal, 0.0));
	
	vec3 direction_to_light_eye = vec3(view_mat * vec4(sun_direction, 1.0));
	float dot_prod = dot(direction_to_light_eye, normal_eye);

	frag_color.rgb *= (global_brightness * brightness_factor * dot_prod) + 0.1;
	
	float cycle_factor = smoothstep(-0.3, 0.2, sun_dot);

	vec4 fog_color = vec4(mix(vec3(0.1, 0.1, 0.1), vec3(0.5, 0.6, 0.75), cycle_factor), 1.0f);

	const float min_fog_radius = 100.0;
	const float max_fog_radius = 800.0;

	// work out distance from camera to point
	// get a fog factor (thickness of fog) based on the distance
	float fog_fac = (tree_dist - min_fog_radius) / (max_fog_radius - min_fog_radius);
	// constrain the fog factor between 0 and 1
	fog_fac = clamp (fog_fac, 0.0, 1.0);

	frag_color = mix (frag_color, fog_color, fog_fac);

	if(tree_dist > tree_dist_far - 100) {
		frag_color.a = mix(frag_color.a, 0, (tree_dist - (tree_dist_far - 100)) / 100);
	} else if(tree_dist < tree_dist_near - 10) {
		frag_color.a = mix(0, frag_color.a, (tree_dist - tree_dist_near) / 10);
	}
}