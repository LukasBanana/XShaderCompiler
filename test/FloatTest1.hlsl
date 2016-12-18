
// HLSL Translator: Floating-point Scanner Test 1
// 05/11/2016


// VERTEX SHADER

#define X

struct S
{
	float val;
};

float f1()
{
	return 1.0;
}

S f2()
{
	return (S)0;
}

float4 VS() : SV_Position
{
	S S_val[5];
	
	uint a = 1;
	float b = 0.0;
	float c = .0;
	float d = 0.;
	float e = 0.f;
	float f = 1.5e+5;
	float g = 1.5e25;
	float h = 1.5E-10F;
	float i = - - - -.1+-2;
	float j = +-+-+-.1;
	float k = .1h;
	float2 l = 1 .xx;
	float3 m = 1X.xxx;
	float4 n = 1. .xxxx;
	float4 o = 1..xxxx;
	float4 p = 1.0.xxxx;
	float4 q = 1 .xxxx.zz.y.xxxx;
	float4 r = float2(0, 1).y.xxx.z.xxxx;
	float4 s = f1().xxxx;
	float4 t = ((S_val))[0].val.xxxx.wwwz;
	float4 u = f2().val.xxxx;
	float4 v = b.xxxx;
	
	return float4(0, 0, 1, 1).zwzw;
}

