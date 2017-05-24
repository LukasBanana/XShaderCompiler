
#version 450

// Vertex coordinate
layout(location = 0) in vec4 position;

layout(std140) uniform Matrices
{
	// World-view-projection matrix
	mat4 wvpMatrix;
};

void main()
{
	// Vertex coordinate transformation
	gl_Position = wvpMatrix * position;
}
