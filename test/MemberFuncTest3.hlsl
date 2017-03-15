
// Member Function Test 3
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
	struct
	{
		float4 GetAmbient()
		{
			return ambient;
		}

		float4 ambient;
	}
	foo;

	float4 Shade(float3 worldPos, float3 normal)
	{
		float3 lightDir = normalize(position - worldPos);
		float NdotL = saturate(dot(lightDir, normal));
		return float4(color * NdotL, 1);
	}

	static Light GetLight()
	{
		Light l0;
		l0.position = (float3)3;
		l0.color = (float3)6;
		l0.foo.ambient = (float4)9;
		return l0;
	}

	float3 position;
	float3 color;
};

float4 PS(VOut i) : SV_Target
{
	Light l;
	l.foo.ambient = 1;
	return i.color + l.foo.GetAmbient();
}


