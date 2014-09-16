#version 420

in vec2 st;
layout (binding = 0) uniform sampler2D tex;
out vec4 frag_color;

void main () {
	frag_color = texture (tex, st);
}