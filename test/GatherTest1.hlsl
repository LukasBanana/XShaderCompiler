Texture2D tex1;
Texture2D tex2;
Texture2D tex3;
Texture2D tex4;
Texture2DArray tex5;
TextureCube tex6;
TextureCubeArray tex7;

SamplerState ss;

float4 main() : SV_Position
{
	float4 f1 = tex1.Gather(ss, float2(0, 0));
	float4 f2 = tex2.Gather(ss, float2(0, 0), int2(3, -3));
	float4 f3 = tex3.GatherRed(ss, float2(0, 0), int2(3, -3));
	float4 f4 = tex4.GatherRed(ss, float2(0, 0), int2(0, 0), int2(0, 1), int2(1, 0), int2(1, 1));
	float4 f5 = tex5.GatherRed(ss, float3(0, 0, 0), int2(3, -3));
	float4 f6 = tex6.GatherBlue(ss, float3(0, 0, 0));
	float4 f7 = tex7.GatherAlpha(ss, float4(0, 0, 0, 0));
	
    return f1 + f2 + f3 + f4 + f5 + f6 + f7;
}
