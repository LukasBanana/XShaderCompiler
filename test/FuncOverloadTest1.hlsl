
// Function Overloading Test 1
// 01/03/2017

Texture2D t0;
Texture2D<float2> t1;
Texture2D<float3> t2;
Texture3D t3;

SamplerState s0;

void f(Texture2D t) {}

void f(Texture2D<float2> t) { int x; }
void f(Texture2D<float3> t) { int y; }

void f(Texture3D t) {}

float4 PS(float lod : LOD) : SV_Target
{
	float l = t0.CalculateLevelOfDetail(s0, lod);
	return 1;
}

