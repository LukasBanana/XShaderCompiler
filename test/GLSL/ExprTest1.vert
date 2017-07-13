
#version 330

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
}
