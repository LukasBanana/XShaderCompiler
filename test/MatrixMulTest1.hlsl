
// Matrix Mul Test 1
// 12/05/2017

cbuffer Matrices {
	float3x3 M3;
	float4x4 M4;
	float4x3 M4x3;
};

void main(float3 v3 : VEC3, float4 v4 : VEC4) {
	
	float3x4 a = (float3x4)0;
	float4x4 b = mul(M4, M4);
	float4x4 c = mul(M4x3, a);
    
	float3 d = mul(M3, v3);
	float4 e = mul(M4, v3);
	float3 f = mul(M3, v4);
	float4 g = mul(M4, v4);
	
	float h = mul(v3, v3);
	float i = mul(v3, v4);
	float j = mul(v4, v3);
	float k = mul(v4, v4);
    
    float4x4 l = M4*1;
    float4x4 m = 2*M4;
    float4x4 n = M4+3;
    float4x4 o = 4+M4;
	
}

