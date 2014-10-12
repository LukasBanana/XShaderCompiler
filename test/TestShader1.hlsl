
// HLSL Translator: Shader Test 1
// 09/10/2014

// Vertex shader

cbuffer VertexParam : register(b0)
{
	float4x4 wvpMatrix : packoffset(c0);
//	nointerpolation float3 normal	: NORMAL : packoffset(c4.x), test : packoffset(c5.x);
};

struct VertexIn
{
	float3 coord	: POSITION;
	float3 normal	: NORMAL;
	float2 texCoord	: TEXCOORD0;
	float4 color	: COLOR;
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
	
	// Vertex transformation
	outp.position	= mul(wvpMatrix, float4(inp.coord, 1.0));
	outp.texCoord	= inp.texCoord;
	
	// Per-vertex lighting
	float3 lightDir = { 0.5, -0.5, 1.0 };
	
	float NdotL		= dot(normalize(inp.normal), -normalize(lightDir));
	float shading	= max(0.2, NdotL);

	outp.color		= inp.color * shading;
	
	return outp;
}

// Pixel shader

Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

float4 PS(VertexOut inp) : SV_Target0
{
	float4 diffuse = tex.Sample(samplerState, inp.texCoord);
	return inp.color * diffuse;
}

