
// Matrix Subscripts Test 1
// 13/05/2017

uniform float4x4 M4;
uniform float2x3 M2x3;

void main() {
	
	float a = M4._m00;
	float3 b = M4._11_33_12;
	float2 c = M4._m00_m11;
	float2 d = M4._11_22;
	float e = M2x3._23;
	
}
