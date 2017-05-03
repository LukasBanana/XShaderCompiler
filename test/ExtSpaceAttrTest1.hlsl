
// Language Extension Test of "space" Attribute
// 19/04/2017

cbuffer Matrices {
	[space(MODEL, PROJECTION)]
	float4x4 wvpMatrix;
	
	[space(MODEL, WORLD)]
	float4x4 wMatrix;
};

struct VIn {
	[space(MODEL)]
	float3 position : POSITION;
	
	[space(MODEL)]
	float3 normal : NORMAL;
};

struct VOut {
	[space(PROJECTION)]
	float4 position : SV_Position;
	
	[space(WORLD)]
	float3 normal : NORMAL;
};

[space(WORLD)]
float4 GetWorldPos([space(MODEL)] float3 v)
{
	return mul(wMatrix, float4(v, 1));
}

void main(VIn i, out VOut o) {
	o.position = mul(wvpMatrix, float4(i.position, 1));
	o.normal = mul(wMatrix, float4(i.normal, 0)).xyz;
	
	// "WORLD" space
	float3 worldPos = mul(wvpMatrix, float4(i.position, 1)).xyz;
	
	// "WORLD" space
	float4 worldPos2 = GetWorldPos(i.position),
	       modelPos = i.position;
	
	//ERROR
	//worldPos2 = modelPos;
}
