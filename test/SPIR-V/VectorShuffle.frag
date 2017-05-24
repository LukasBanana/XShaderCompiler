
#version 450

layout(std140) uniform Material {
	vec4 diffuse;
};

layout(location = 0) out vec4 color;

void main() {
	color = vec4(diffuse.rgb, 1);
}
