
// Reflection Test 1
// 10/06/2017

#define BAR

cbuffer Buf2 : register(b2) {
	float4x4 wvpMatrix;
};

uniform float4x4 wMatrix;

Texture2D tex1 : register(t1);

[numthreads(1, 2, 3)]
void main() {
	tex1;
	wvpMatrix;
	wMatrix;
}
