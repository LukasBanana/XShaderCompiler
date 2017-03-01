
// Intrinsic Test 1
// 01/03/2017

float4 VS(float3 dir : DIRECTION) : SV_Position
{
    float y = max(1, dir);
    return y;
}

