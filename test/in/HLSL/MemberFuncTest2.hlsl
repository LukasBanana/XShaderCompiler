
// Member Function Test 2
// 15/03/2017

struct Light {
	float4 Shade(float3 worldPos, float3 normal) {
		float3 lightDir = normalize(position - worldPos);
		float NdotL = saturate(dot(lightDir, normal));
		return float4(color * NdotL, 1);
	}
	
	static Light Get() {
		Light l0 = { (float3)3, (float3)9 };
		return l0;
	}
	
	float3 position;
	float3 color;
};

struct VIn {
	float3 worldPos : WORLDPOS;
	float3 normal : NORMAL;
};

Light GetLight1() {
	Light l = (Light)0;
	return l;
}

typedef Light LightArray[2];

LightArray GetLight2() {
	Light list[2] = { GetLight1(), GetLight1() };
	return list;
}

float4 main(VIn i) : LIGHTCOLOR {
	#if 1
	
	//return Light::Get().Shade(i.worldPos, i.normal);
	return GetLight1().Shade(i.worldPos, i.normal);
	//return GetLight2()[0].Shade(i.worldPos, i.normal);
	
	#else
	return 1;
	#endif
}


