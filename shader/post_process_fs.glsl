#version 420

in vec2 texcoord;

// texture sampler
layout (binding = 0) uniform sampler2D tex;
layout (binding = 1) uniform sampler2D depth;
layout (binding = 2) uniform sampler2D hdr_low;
layout (binding = 3) uniform sampler2D hdr_high;

// size of 1 pixel in texture coordinates
uniform vec2 pixel_scale;

// position of sun in screen space
uniform vec2 sun_pos;

// sun direction vector
uniform vec3 sun_dir;

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



vec4 bloom(vec4 colour) {

	vec2 size = vec2(1);
	int samples = 5; // pixels per axis; higher = bigger glow, worse performance
	float quality = 2.5; // lower = smaller glow, better quality

	vec4 source = texture(tex, texcoord);
	vec4 sum = vec4(0);
	int diff = (samples - 1) / 2;
	vec2 sizeFactor = vec2(1) / size * quality;
  
	for (int x = -diff; x <= diff; x++) {
		for (int y = -diff; y <= diff; y++) {
			vec2 offset = vec2(x, y) * sizeFactor;
			sum += texture(tex, texcoord + offset);
		}
	}
  
	return ((sum / (samples * samples)) + source) * colour;
}

vec4 gauss_blur(sampler2D src, vec2 pos, float mul) {

	vec4 color;

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


vec4 godray(sampler2D src, vec2 pos) {

	const float density = 0.8;
	const int samples = 32;
	const float weight = 0.3;
	const float decay = 1.01;
	
	vec2 deltaTexCoord = (pos - sun_pos);
	
	deltaTexCoord *= 1.0f / samples * density;
	
	vec3 col = texture(src, pos).rgb;
//	if((col.r + col.g + col.b)/3 < threshold) return vec4(0);

	float illuminationDecay = 1.0f;

	vec2 uv2 = texcoord;
	
	for (int i = 0; i < samples; i++) {
		uv2 -= deltaTexCoord;
		col += texture(src, uv2).rgb * illuminationDecay * weight;
		illuminationDecay *= decay;
	}
	
	return vec4(col * 0.04, 1) * 1;
}


void main () {

	// only blur rhs for comparison
	if (false && texcoord.x >= 0.5) {
		frag_color = texture(hdr_low, texcoord);
	} else {
  //	frag_color = bloom(vec4(0.1));
		frag_color = texture(tex, texcoord) + gauss_blur(hdr_low, texcoord, 1.5) + godray(hdr_high, texcoord) * 4;
	}

	clamp(frag_color, 0.0, 1.0);
}