
// HLSL Translator: Shader Test 1
// 09/10/2014

// Vertex shader

struct ParamStruct
{
	int param;
};

cbuffer VertexParam : register(b0)
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
	float4 v4;
};

TestStruct Get_TestStruct()
{
	TestStruct s;
	s.mat = (float4x4)0;
	s.v4 = (float4)0;
	return s;
}

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
	outp.position	= mul(wvpMatrix, float4(inp.coord, 1.0));//, 1, foo, bar);
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

// Pixel shader
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
};

void Frustum(inout float4 v)
{
	v.x = v.x*0.5 + 0.5;
	v.y = v.y*0.5 + 0.5;
}

// 3rd forward declaration (must not override the function reference in the DAST)
void Frustum(inout float4 v);

// Compute shader

struct ComputeIn
{
	uint3 threadID : SV_DispatchThreadID;
	uint groupIndex : SV_GroupIndex;
};

int test(int x){return 0;}
void test2(int x, inout const row_major float4x4 y){}


[numthreads(10, 1, 1)]
void CS(uint3 threadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
//void CS(ComputeIn inp)
{
	int z = 0;
	// expression tests
	float x = 3 * (float)-threadID.x;
	int y = (int)x * 2 + 2 - (int)(x + 0.5) + (int)(float)(z) + 9;
	float a = 1, b = 2 + (a += 4);
	
	//float4x4 m = wvpMatrix;
	
	struct
	{
		int x, y;
	}
	s1;
	
	InnerStruct s2;
	
	float3 v0;
	float3 v1 = float3(1, 2, 3) + v0.xxy + Get_TestStruct().v4.zyx + float4(0).xyz * 2;
	
	//#if 1
	// requires GLSL 1.30 or the "GL_EXT_gpu_shader4" extension.
	//int mask = 0xffff | y;
	int mask = 256 | y;
	//#endif
	
	// Expression statements
	5 + 2, ++mask, mask <<= 2;
	const int cnst0 = 0;
	int cnst1 = 1;
	row_major float4x4 cnst2;
	
	// Loop test
	[unroll(4)]
	for (int i = 0; i < 10; ++i)
		for (int y = 0; y < 20; y++, ++mask)
		{
			// Conidition test
			[branch]
			if (x > y + 2)
				i++, --i, threadID.x++;
			else if (!(x == 2))
			{
				int y;
				i += 4, ++i, i *= 2;
				y = 2, y = 4;
				break;
			} else { int z; x = y; }
		}
	
	while (test(x))
		do {
			float4x4 mat0;
			test2(y, mat0);
			do
				float4 v = 0;
			while (v.x < 10);
		} while ((bool)(x) == true);
	
	// Switch test
	switch ((int)x, mask)
	{
		case 1:
		{
			int x = 5;
			;
			;;;
			{
				;;;
			}
		}
		break;
		case 2:
			break;
		default:
			break;
	}
	
	//...
}

