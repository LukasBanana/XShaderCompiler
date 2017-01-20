
// I/O Function Call Test 1
// 19/01/2017


float4 f(float x = 1, float y = 2, float z = 3)
{
    return float4(x, y, z, 1);
}

float4 f(int x)
{
    return (float4)x;
}

float4 VS() : SV_Position
{
    return f(5) + f();
}

