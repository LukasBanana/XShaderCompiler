
// HLSL Translator: Expression Test 2
// 14/11/2016

struct S1
{
	float x, y, z;
};

void CS()
{
	S1 s1 = (struct S1)0;
	
	S1 s2 = (S2)0;
	
	int i1 = (int).0f;
	
	float f1 = ((float3)1).x;
	
	float f2 = ((vector<float, (1+4)/5+3>)1).w;
	
}

