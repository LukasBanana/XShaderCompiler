
// Texture Objects Test
// 07/12/2016

#define SAMPLE_MULTIPLIER (2)

SamplerState smpl0 : register(s0);

SamplerState smpl1
{
	Filter = ANISOTROPIC;
	AddressU = MIRROR;
	AddressV = CLAMP;
//	BorderColor = { 1, 2, 3, 4 };
	BorderColor = float4(2, 4, 0, 1);
//	BorderColor = (float4)1;
};

Texture2D tex0 : register(t0);

Texture2D<int4> tex1 : register(t1);

Texture2DMS<float4, 32*SAMPLE_MULTIPLIER> tex2 : register(t2);

struct VOut
{
	float2 texCoord : TEXCOORD;
};

SamplerState GetSmpl(SamplerState s)
{
	return s;
}

Texture2D<float4> GetTex(Texture2D<float4> t)
{
	return t;
}

float3 SampleNormal(Texture2D tex, float2 tc)
{
	return tex.Sample(GetSmpl(smpl0), tc).rgb * 2.0 - 1.0;
}

//float4 PS(float2 texCoord : TEXCOORD) : SV_Target
float4 PS(VOut inp) : SV_Target
{
	float4 c = (float4)0;
	
	float3 normal = SampleNormal(GetTex(tex0), inp.texCoord);
//	float3 normal = SampleNormal(tex0, inp.texCoord);
	
	// Sample
	c += tex0.Sample(smpl0, inp.texCoord);
	c += tex0.Sample(smpl0, inp.texCoord, 0);
	
	// Error: "Sample" not supported for non-floating-point texture formats
	c += tex1.Sample(smpl0, inp.texCoord);
	
	// Load
	c += tex2.Load((int2)inp.texCoord, 0);
	
	// SampleLevel
	c += tex0.SampleLevel(smpl0, inp.texCoord, 0.0);
	c += tex0.SampleLevel(smpl0, inp.texCoord, 0.0, 0);
	c += tex0.SampleLevel(smpl0, inp.texCoord, 0.0, int2(1, 2));
	
	return c;
}

