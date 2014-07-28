#version 420

in vec2 texcoord;

layout (binding = 0) uniform sampler2D tex0;

out vec4 frag_color;

void main() {
//	frag_color = texture(tex0, texcoord.st);
	frag_color = vec4(0.0, 1.0, 0.0, 0.7);

	if(frag_color.a == 0) discard;
}