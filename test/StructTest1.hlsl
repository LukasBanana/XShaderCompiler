
// Struct Test 1
// 01/12/2016

struct S1
{
	struct S1_1
	{
		float x;
	}
	s1_1;
};

float4 PS() : SV_Target
{
	S1 s1;
	
	return (float4)0;
}


