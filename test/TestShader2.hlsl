
// HLSL Translator: Shader Test 2
// 27/10/2014

// Vertex shader

cbuffer VertexParam : register(b0)
{
	float4x4 wvpMatrix;
};

struct VertexIn
{
	float3 coord	: POSITION;
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
	VertexOut outp;// = (VertexOut)0;
	
	// Vertex transformation
	outp.position	= mul(wvpMatrix, float4(inp.coord, 1.0));
	outp.texCoord	= inp.texCoord;
	
	// Per-vertex lighting
	float3 lightDir = { 0.5, -0.5, 1.0 };
	
	float3 normal	= normalize(inp.normal);
	
	float NdotL		= dot(normal, -normalize(lightDir));
	float shading	= max(0.2, NdotL);
	
	outp.color		= float4((float3)shading, 1.0);
	
	return outp;
}

// Pixel shader

Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

/*float4 PS(VertexOut inp) : SV_Target0
{
	return inp.color * tex.Sample(samplerState, inp.texCoord);
}*/

struct PixelOut
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
	float depth : SV_Depth;
};

PixelOut PS(VertexOut inp)
{
	PixelOut outp;
	
	outp.color0 = (float4)1.0;
	outp.color1 = (float4)1.0;
	outp.depth = inp.position.z;
	
	return outp;
}



