// Semantic Test 5
// 03/04/2017

struct Input
{
    float4 color;
};

struct S
{
	Input c0 : TEXCOORD0;
	Input c1 : TEXCOORD2;
};

float4 main(S i) : SV_Target
{
	return i.c0.color + i.c1.color;
}
