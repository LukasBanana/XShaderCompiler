
struct PSInput
{
	float4 v_uvs_0[2]: TEXCOORD1;
};

float4 PSMain(PSInput input) : SV_TARGET
{
	return input.v_uvs_0[0].xxxx;
}

