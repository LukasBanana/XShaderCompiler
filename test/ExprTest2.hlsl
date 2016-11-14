
// HLSL Translator: Expression Test 2
// 14/11/2016

struct S1
{
	float x, y, z;
};

void CS()
{
	S1 s1 = (struct S1)0;
	
	int i1 = (int).0f;
	
	float f1 = ((float3)1).x;
	
}

