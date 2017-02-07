
// HLSL Translator: Suffix Test 1
// 07/02/2017

struct S
{
    float val;
};

float f1()
{
    return 0;
}

S f2()
{
    return (S)0;
}

float4 VS() : SV_Position
{
    int x[(1).xxx.zy]; //INCOMPLETE!
    float  a = 0;
    float3 b = f1().xxx;
    float3 c = f1().xxx.xyz;
    float3 d = a.xxx;
    float  e = a.xxx.x;
    float3 f = a.x.xxx;
    float3 g = f2().val.xxx;
    return 0;
}

