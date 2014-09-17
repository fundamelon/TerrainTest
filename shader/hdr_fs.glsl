#version 400

in vec2 texcoord;

layout (binding = 0) uniform sampler2D tex;
layout (binding = 1) uniform sampler2D depth;

uniform float hdr_threshold;
uniform vec2 sun_pos;


out vec4 frag_color;

// Gaussian kernel weights
#define KERNEL_SIZE 25
float kernel_weights[] = float[]( 
  0.00048031, 0.00500493, 0.01093176, 0.00500493, 0.00048031,
  0.00500493, 0.05215252, 0.11391157, 0.05215252, 0.00500493,
  0.01093176, 0.11391157, 0.24880573, 0.11391157, 0.01093176,
  0.00500493, 0.05215252, 0.11391157, 0.05215252, 0.00500493,
  0.00048031, 0.00500493, 0.01093176, 0.00500493, 0.00048031
);
float weights_factor = 1.01238;


float brightness(vec4 col) {
	
	const vec4 dot_fac = vec4(1.0);
	return dot(col.rgb, dot_fac.rgb) / 3.0;
}

vec4 gauss_blur(sampler2D src, vec2 pos, float mul) {

	vec4 color;
	vec2 pixel_scale = vec2(1/1200, 1/600);

	vec2 offset[] = vec2[](
		vec2 (-pixel_scale.s * 2.0, -pixel_scale.t * 2.0),
		vec2 (-pixel_scale.s, -pixel_scale.t * 2.0),
		vec2 (0.0, -pixel_scale.t * 2.0),
		vec2 (pixel_scale.s, -pixel_scale.t * 2.0),
		vec2 (pixel_scale.s * 2.0, -pixel_scale.t * 2.0),
  
		vec2 (-pixel_scale.s * 2.0, -pixel_scale.t),
		vec2 (-pixel_scale.s, -pixel_scale.t),
		vec2 (0.0, -pixel_scale.t),
		vec2 (pixel_scale.s, -pixel_scale.t),
		vec2 (pixel_scale.s * 2.0, -pixel_scale.t),
  
		vec2 (-pixel_scale.s * 2.0, 0.0),
		vec2 (-pixel_scale.s, 0.0),
		vec2 (0.0, 0.0),
		vec2 (pixel_scale.s, 0.0),
		vec2 (pixel_scale.s * 2.0, 0.0),
  
		vec2 (-pixel_scale.s * 2.0, pixel_scale.t),
		vec2 (-pixel_scale.s, pixel_scale.t),
		vec2 (0.0, pixel_scale.t),
		vec2 (pixel_scale.s, pixel_scale.t),
		vec2 (pixel_scale.s * 2.0, pixel_scale.t),
  
		vec2 (-pixel_scale.s * 2.0, pixel_scale.t * 2.0),
		vec2 (-pixel_scale.s, pixel_scale.t * 2.0),
		vec2 (0.0, pixel_scale.t * 2.0),
		vec2 (pixel_scale.s, pixel_scale.t * 2.0),
		vec2 (pixel_scale.s * 2.0, pixel_scale.t * 2.0)
	);

	for (int i = 0; i < KERNEL_SIZE; i++) {
	    color += texture (src, pos + offset[i]*mul) * kernel_weights[i] * weights_factor;// * (1 - texture(depth, texcoord).rgb);
	}

	return color;
}


void main() {

	vec4 color = gauss_blur(tex, texcoord, 1.0);
	vec4 depth_val = texture(depth, texcoord);
	float dist = distance(texcoord, sun_pos);
	if(dist > 0.8 || (brightness(depth_val) != 1.0)) discard;

	float dist_mul = smoothstep(0.8, 0, dist);
	
	frag_color = clamp(color * dist_mul, 0.0, 1.0);
}