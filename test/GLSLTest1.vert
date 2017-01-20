
// GLSL Test 1
// Vertex Shader
// 20/01/2017

#version 140

void f()
{
	gl_Position += vec4(1);
}

void main()
{
	gl_VertexID += 1;
	gl_Position = vec4(0);
	f();
	gl_Position = gl_Position.wzyx;
}
