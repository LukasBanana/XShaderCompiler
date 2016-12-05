// Error Test 1
// 05/11/2016

cbuffer Matrices : register(b0)
{
	float4x4 wvpMatrix;
};

struct Light
{
	float3 position;
	float3 color;
};


float4 VertexMain(float3 position : POSITION) : SV_Position
{
	Light light;
	float3 color = (float3)0.5 + light;
	
	return mul(wvpMatrix, position);
}
