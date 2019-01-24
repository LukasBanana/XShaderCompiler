
// Preprocessor Test 3
// 20/06/2017

#version 130

#define DECL_VECTOR4(X) uniform vec4 vector_##X

DECL_VECTOR4(__EVAL__(1)); // <-- BUG with semicolon when "__EVAL__" is used!!!

//#include "PPTest3Header.vert"

void main() {
	gl_Position = vector_0 + vector_1;
}
