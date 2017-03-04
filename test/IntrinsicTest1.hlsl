
// Intrinsic Test 1
// 01/03/2017

float4 VS(
    float3 dir : DIRECTION,
    float s : SIN,
    float c : COS) : SV_Position
{
    float a = max(1, dir).z;
    int b = firstbithigh(3.5);
    
    sincos(a, s, c);
    
    InterlockedAdd(a, s, c);
    
    return a;
}

