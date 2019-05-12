// Source: https://github.com/LukasBanana/XShaderCompiler/issues/94

struct PSInput
{
    float4 uv : TEXCOORD0;
};

#define INPUT_UV input.uv SWIZZLE2_0
#define SWIZZLE2_0 .xy

float4 main(PSInput input) : SV_Position
{
    float2 sampledInput = INPUT_UV;
	return sampledInput.xyxy;
}
