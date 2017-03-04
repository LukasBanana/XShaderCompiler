
// Vector Compare Test 1
// 04/03/2017

struct F
{
    float x, y;
};

struct S
{
    float foo;
    float2 pos;
    float2 dir;
    float bar;
    F f;
    int x;
};

float4 VS() : SV_Position
{
    S s = (S)0;
    
    #if 0
    float3 a = 1;
    float3 b = (1 + a).zyx;
    float3 c = (a + 1).zyx;
    #endif
    
    //sincos(s.x, s.foo, s.bar);
    
    bool4 y = (s.pos >= s.dir).yxxy;
    
    float4 x = ((s.pos < s.dir) ? 1 : 2).xyxy;
    //float3 y = (1 < 2 ? 1 : a).zyx;
    
    //return s.pos.xyxy;
    return 0;
}

