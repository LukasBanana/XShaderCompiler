
// RWTexture Test 1
// 13/03/2017

RWTexture2D<int> tex[2];

float4 main() : SV_Position
{
    float4 a = (tex[0])[int2(1, 2)]; // must be "imageLoad(tex[0], ivec2(1, 2))" in GLSL
    float4 b =  tex[0] [int2(3, 4)]; // must be "imageLoad(tex[0], ivec2(1, 2))" in GLSL
    
    return 1;
}
