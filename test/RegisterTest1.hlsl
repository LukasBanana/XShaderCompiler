
// Register Test 1
// 02/12/2016

#ifdef TEST_ERRORS
cbuffer Matrices : register(vs, b0) : register(ps, b1)
#else
cbuffer Matrices : register(b1)
#endif
{
	float4 diffuse;
};

//Buffer<float4> buffer0 : register(t0);

Texture2D tex0 : register(vs, t[1])
               : register(ps, t1[2]);

sampler2D tex1 = sampler_state
{
	texture = <tex0>;
	MINFILTER = LINEAR;
};

SamplerState smpl0 : register(ps_5_0, s0)
                   : register(z[12]); // this register is ignored

SamplerState smpl1 : register(s1)
{
	MINFILTER = POINT;
};

float4 PS(float2 texCoord : TEXCOORD) : SV_Target : SV_Target5
{
	tex0 = tex0;
	//return tex0.Sample(smpl0, texCoord);
	return diffuse;
}


