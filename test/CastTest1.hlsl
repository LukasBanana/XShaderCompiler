
// Type Cast Test 1
// 01/12/2016

// Type Cast Rules for HLSL:
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb172396(v=vs.85).aspx


struct S1
{
	float x, y;
};

struct S2
{
	float a;
	S1 b;
};


float4 VS() : SV_Position
{
	S2 a = (S2)0;
	
	
	return (float4)1;
}
