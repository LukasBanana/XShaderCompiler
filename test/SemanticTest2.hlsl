
// I/O Semantic Test 2
// 26/12/2016

cbuffer Matrices : register(b0)
{
	float4x4 wvpMatrix;
	float4x4 wMatrix;
};

struct VOut
{
	float3 normal : NORMAL;
};

void VS(
	float3 pos : POSITION,
	float2 tc : TC,
	float3 normal : NORMAL,
	out VOut outp,
	out float2 texCoord : TEXCOORD,
	out float4 tpos : SV_Position)
{
	tpos = mul(wvpMatrix, pos);
	outp.normal = mul(wMatrix, float4(normal, 0)).xyz;
	texCoord = tc;
}

void PS(
	float3 norm : NORMAL,
	float2 tc : TEXCOORD,
	out float4 color : SV_Target)
{
	float3 normal = normalize(norm);
	float NdotL = dot(normal, float3(0, 0, -1));
	float4 diffuse = tc.xyxy;
	color = diffuse * NdotL;
}



