
// HLSL Translator: Shader Test 1
// 09/10/2014


// VERTEX SHADER

struct ParamStruct
{
	int param;
};

cbuf fer VertexParam : register(b0)
{
	float4x4 wvpMatrix;
	nointerpolation float3 normal[3][2]	: NORMAL, test3;
	struct dataStruct { float2 v0, v1; int2 v2; } data[10];
	ParamStruct param0;
};

cbuffer PixelParam : register(b1)
{
	float4 ambientColor;
};

struct TestStruct
{
	float4x4 mat;
};

struct VertexIn
{
	float3 coord	: POSITION;
	float3 normal	: NORMAL;
	float2 texCoord	: TEXCOORD0;
	float4 color	: COLOR;
	//TestStruct test;
};

struct VertexOut
{
	float4 position	: SV_Position;
	float2 texCoord	: TEXCOORD0;
	float4 color	: COLOR;
};

float3 GammaCorrect(float3 color, float gamma)
{
	return pow(color, 1.0/gamma);
}

VertexOut VS(VertexIn inp, uint vertexID : SV_VertexID)
{
	VertexOut outp = (VertexOut)0;
	
	TestStruct test = (TestStruct)0;
	
	// Vertex transformation
	outp.position	= mul(wvpMatrix, float4(inp.coord, 1.0));
	outp.texCoord	= inp.texCoord;
	
	// Per-vertex lighting
	float3 lightDir = (float3)0.5;//{ 0.5, -0.5, 1.0 };
	float3 lightDir2 = { 0.5, -0.5, 1.0 };
	
	float3 normal = normalize(inp.normal);
	
	if (vertexID < 3)
	{
		float NdotL		= dot(normal, -normalize(lightDir));
		float shading	= max(0.2, NdotL);
	}
	
	outp.color		= float4(GammaCorrect(inp.color.xyz, 1.2), 1.0);
	
	return outp;
}


// PIXEL SHADER

Texture2D tex : register(t0), tex2 : register(t1);
Texture2D<float> tex3 : register(t2);
SamplerState samplerState : register(s0);

// 1st forward declaration
void Frustum(inout float4 v);
// 2nd forward declaration
void Frustum(inout float4 v);

// overloaded function
void Frustum(inout float4 v, int x) {}

// "SamplerState ss" must be removed by the translator
void TexTest(Texture2D t2d, SamplerState ss) {}

float4 PS(VertexOut inp) : SV_Target0
{
	float3 interpColor = float3(1.0, 0.0, 0.0);
	
	float4 diffuse = lerp(
		(float4)1.0,
		saturate(tex.Sample(samplerState, inp.texCoord)),
		inp.position.x
	);
	
	// dFdxCoarse requires GLSL 4.00 or the "GL_ARB_derivative_control" extension
	float2 tc_dx = ddx_coarse(inp.texCoord);
	
	float4 viewRay = (float4)0.0;
	Frustum(viewRay);
	
	// "samplerState" must be removed by the translator
	TexTest(tex, ((samplerState)));
	
	int intrl;
	InterlockedAdd(intrl, 5);
	
	return ambientColor + saturate(inp.color * diffuse);
}



