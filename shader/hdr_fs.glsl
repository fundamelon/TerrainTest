#version 400

in vec2 texcoord;

layout (binding = 0) uniform sampler2D tex;

uniform float hdr_threshold;

out vec4 frag_color;

float brightness(vec4 col) {
	
	return (col.r + col.g + col.b) / 3.0;
}


void main() {

	vec4 color = texture(tex, texcoord);
	if(brightness(color) <= hdr_threshold) discard;
	
	frag_color = color - hdr_threshold;
}