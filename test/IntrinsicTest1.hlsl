
// Intrinsic Test 1
// 01/03/2017

cbuffer Buf
{
    float3 dir;
};

groupshared int g_Value;

[numthreads(1, 1, 1)]
void main()
{
    float a = max(1, dir).z;
    int b = firstbithigh(3.5);
    
	float s = dir.x;
	float c = dir.y;
    sincos(a, s, c);
    
    InterlockedAdd(g_Value, s);
}

