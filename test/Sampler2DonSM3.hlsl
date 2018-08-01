
struct Input
{
    float4 pos : POSITION;
    float2 texcoord : TEXCOORD0;
};

uniform float4 SampleDelta : register(c0);
uniform float4 Sharpness : register(c1);

uniform sampler2D Ssao : register(s0);

float4 main(Input input) : COLOR
{
    float4 uv = float4(input.texcoord * Sharpness.xy + SampleDelta.xy, 0, 0);
    return tex2Dlod(Ssao, uv);
}
