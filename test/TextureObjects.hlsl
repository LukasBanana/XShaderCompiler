
// Texture Objects Test
// 07/12/2016

#define SAMPLE_MULTIPLIER (2)

SamplerState smpl0 : register(s0);

SamplerState smpl1
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = MIRROR;
	AddressV = CLAMP;
};

Texture2D tex0 : register(t0);

Texture2D<int4> tex1 : register(t1);

Texture2DMS<float4, 32*SAMPLE_MULTIPLIER> tex2 : register(t2);

struct VOut
{
	float2 texCoord : TEXCOORD;
};

//float4 PS(float2 texCoord : TEXCOORD) : SV_Target
float4 PS(VOut outp) : SV_Target
{
	float4 c = (float4)0;
	
	c += tex0.Sample(smpl0, outp.texCoord);
	c += tex1.Sample(smpl0, outp.texCoord);
	c += tex2.Load((int2)outp.texCoord, 0);
	
	return c;
}

