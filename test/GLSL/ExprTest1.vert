
#version 330

layout(early_fragment_tests) in;

//layout(binding = 1)
uniform vec4 diffuse;
uniform sampler2D tex0, tex1;

struct Buffer { int x; };

layout(std140)
uniform Cbuffer {
	vec4 a, b, c;
	struct { int foo; } x; //ERROR
};

void main() {
	gl_Position = vec4(1);
	vec4 c = vec4(0);
	c += diffuse;
}
