
// HLSL Translator: Type Test 2
// 27/11/2016

typedef struct
{
	float x, y;
}
S, S_2[2];

typedef S_2 S_2_3[3];

float4 VS() : SV_Position
{
	S a;
	S_2 b;
	S_2_3 c;
	S_2_3 d[4];
	
	return (float4)1;
}


