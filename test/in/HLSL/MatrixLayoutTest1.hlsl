
// Matrix Storage Layout Test 1
// 29/04/2017

cbuffer Matrices {
	row_major    float4x4 M0;
	column_major float3x4 M1;
	             float2x4 M2;
};

struct In {
	float4 v4 : V4IN;
	float3 v3 : V3IN;
};

struct Out {
	float4 v4 : V4OUT;
	float3 v3 : V3OUT;
};

void main(In i, out Out o) {
	
	o.v4 = mul(M0, i.v4);
	
}

