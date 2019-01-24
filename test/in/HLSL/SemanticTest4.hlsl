
struct VIn
{
	float3 coord : COORD;
	float2 texCoord : TEXCOORD0;
	/*nointerpolation*/ float2 texCoord2 : TEXCOORD1;
	/*noperspective*/ float3 normal : NORMAL;
};

float4 VS(VIn inp) : SV_Position
{
	return 1;
}
