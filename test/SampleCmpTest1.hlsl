Texture2D tex1;
TextureCubeArray tex2;

SamplerComparisonState ss;

float4 main() : SV_Position
{
	float f1 = tex1.SampleCmp(ss, float2(1, 2), 0.5f);
	float f2 = tex1.SampleCmp(ss, float2(1, 2), 0.5f, int2(3, -3));
	float f3 = tex2.SampleCmp(ss, float4(1, 2, 3, 4), 0.5f);
	
	float f4 = tex1.SampleCmpLevelZero(ss, float2(1, 2), 0.5f);
	float f5 = tex1.SampleCmpLevelZero(ss, float2(1, 2), 0.5f, int2(3, -3));
	
    return f1 + f2 + f3 + f4 + f5;
} 