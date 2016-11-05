
// HLSL Translator: Shader Test 1
// 09/10/2014


// VERTEX SHADER

#define FOREVER for(;;)

cbuffer Settings : register(b0)
{
	float4x4	wvpMatrix;
	float4x4	wMatrix;
	float4		diffuse;
};

struct VertexIn
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float2 texCoord	: TEXCOORD0;
};

struct VertexOut
{
	float4 position	: SV_Position;
	float2 texCoord	: TEXCOORD0;
	float4 color	: COLOR;
};

VertexOut VS(VertexIn inp)
{
	VertexOut outp = (VertexOut)0;
	
	//FOREVER
	for(;;)
	{
		
	}
	
	return outp;
}




