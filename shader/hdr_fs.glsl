#version 400

in vec2 texcoord;

layout (binding = 0) uniform sampler2D tex;
layout (binding = 1) uniform sampler2D depth;

uniform float hdr_threshold;
uniform vec2 sun_pos;


out vec4 frag_color;

// Gaussian kernel weights
#define KERNEL_SIZE 25
const float kernel_weights[] = float[]( 
  0.00048031, 0.00500493, 0.01093176, 0.00500493, 0.00048031,
  0.00500493, 0.05215252, 0.11391157, 0.05215252, 0.00500493,
  0.01093176, 0.11391157, 0.24880573, 0.11391157, 0.01093176,
  0.00500493, 0.05215252, 0.11391157, 0.05215252, 0.00500493,
  0.00048031, 0.00500493, 0.01093176, 0.00500493, 0.00048031
);
const float weights_factor = 1.01238;

// TODO: change this crap to a two-pass horizontal/vertical blur

vec4 gauss_blur(sampler2D src, vec2 pos, float mul) {

	vec4 color;
	const vec2 pixel_scale = vec2(1/1200, 1/600);

	const vec2 offset[] = vec2[](
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


float brightness(vec4 col) {
	
	const vec4 dot_fac = vec4(1.0);
	return dot(col.rgb, dot_fac.rgb) / 3.0;
}


void main() {

	vec4 depth_val = texture(depth, texcoord);
	float dist = distance(texcoord, sun_pos);
	if(dist > 0.8 || (brightness(depth_val) < 1.0)) discard;

	vec4 color = texture(tex, texcoord);
	
	frag_color = clamp(color * smoothstep(0.8, 0, dist), 0.0, 1.0);
}