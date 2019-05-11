
struct VIn
{
	float3 coord : COORD;
	float2 texCoord : TEXCOORD0;
	float3x3 wMatrix : MATRIX;
	nointerpolation float2 texCoord2 : TEXCOORD1;
	noperspective float3 normal : NORMAL;
};

float4 VS(VIn inp, nointerpolation uint id : SV_VertexID, out nointerpolation float x : FOO) : SV_Position
{
    x = 2;
	return 1;
}
