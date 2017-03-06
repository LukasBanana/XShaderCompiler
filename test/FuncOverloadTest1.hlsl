
// Function Overloading Test 1
// 01/03/2017

Texture2D t0;
Texture2D<float2> t1;
Texture2D<float3> t2;
Texture3D t3;

SamplerState s0;

void f(Texture2D t) { int a; }
void f(Texture2D<float2> t) { int b; }
void f(Texture2D<float3> t) { int c; }
void f(Texture3D t) { int d; }

float4 PS(float lod : LOD) : SV_Target
{
	f(t0);
	f(t1);
	f(t2);
	f(t3);
	float l = t0.CalculateLevelOfDetail(s0, lod);
	return 1;
}

