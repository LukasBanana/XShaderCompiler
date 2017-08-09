
// ESSL I/O Semantics Test 1
// 09/08/2017

struct Input
{
	float3 worldPos : WORLDPOS;
};

struct Output
{
    float4 color0 : SV_Target0;
    float4 color1 : SV_Target1;
    float4 color2 : SV_Target2;
};

Output main(Input i)
{
	Output o;
    return o;
}

