// Geometry Test 2
// 16/01/2017

cbuffer Matrices
{
	float4x4 wMatrix;
	float4x4 vpMatrix;
};

// Vertex Shader

struct VIn
{
	float3 position : POSITION;
	float size : SIZE;
};

struct VOut
{
	float4 worldPos : WORLDPOS;
	float worldSize : WORLDSIZE;
};

VOut VS(VIn inp)
{
	VOut outp = (VOut)0;
	outp.worldPos = mul(wMatrix, float4(inp.position, 1));
	outp.worldSize = inp.size;
	return outp;
}

// Geometry Shader

struct GOut
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

/*
Generate center-aligned quad from point:
1------3
| \    |
|   \  |
|     \|
0------2
*/
[maxvertexcount(4)]
void GS(point VOut inp[1], inout TriangleStream<GOut> stream)
{
	float s = inp[0].worldSize;
	GOut o = (GOut)0;
	
	// 1st output vertex
	o.position = inp[0].worldPos + float4(-s, -s, 0, 0);
	o.texCoord = float2(0, 1);
	stream.Append(o);
	
	// 2nd output vertex
	o.position = inp[0].worldPos + float4(-s,  s, 0, 0);
	o.texCoord = float2(0, 0);
	stream.Append(o);
	
	// 3rd output vertex
	o.position = inp[0].worldPos + float4( s, -s, 0, 0);
	o.texCoord = float2(1, 1);
	stream.Append(o);
	
	// 4th output vertex
	o.position = inp[0].worldPos + float4( s,  s, 0, 0);
	o.texCoord = float2(1, 0);
	stream.Append(o);
}

// Pixel Shader

Texture2D colorMap : register(ps, t0);
SamplerState linearSampler : register(ps, s0);

float4 PS(GOut inp) : SV_Target
{
	return colorMap.Sample(linearSampler, inp.texCoord);
}


