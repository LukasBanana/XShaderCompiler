
#version 330

//layout(binding = 1)
uniform sampler2D tex0, tex1;

struct Buffer { int x; };

uniform Buffer {
	vec4 a, b, c;
};

void main() {
	gl_Position = vec4(1);
}
