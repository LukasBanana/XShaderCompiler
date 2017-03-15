
// Member Function Test 2
// 15/03/2017

struct VOut
{
	float4 position : SV_POSITION;
	float3 worldPos : WORLDPOS;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

struct Light
{
	float4 Shade(float3 worldPos, float3 normal)
	{
		float3 lightDir = normalize(position - worldPos);
		float NdotL = saturate(dot(lightDir, normal));
		return float4(color * NdotL, 1);
	}

	static Light GetLight()
	{
		Light l0 = { (float3)3, (float3)9 };
		return l0;
	}

	float3 position;
	float3 color;
};

float4 PS(VOut i) : SV_Target
{
	return i.color * Light::GetLight().Shade(i.worldPos, i.normal);
}


