
// Texture Objects Test
// 07/12/2016

SamplerState smpl0 : register(s0);

SamplerState smpl1
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = MIRROR;
	AddressV = CLAMP;
};

Texture2D tex0 : register(t0);

Texture2D<int4> tex1 : register(t1);

Texture2DMS<float4, 32> tex2 : register(t2);


float4 PS(float2 texCoord : TEXCOORD) : SV_Target
{
	float4 c = (float4)0;
	
	c += tex1.Sample(smpl0, texCoord);
	c += tex2.Sample(smpl1, texCoord);
	
	return c;
}

