
// 'clip'-Intrinsic Test
// 07/12/2016

cbuffer Colors : register(b0)
{
	float4 diffuse;
};

float4 PS() : SV_Target
{
	// Alpha test
	clip(diffuse.a - 0.5);
	
	// Dummy vector alpha test
	clip(diffuse.rgb - (float3)0.5);
	
	// Listed expression test
	clip(diffuse.r), clip((uint)(diffuse.a - 0.5));
	
	return diffuse;
}
