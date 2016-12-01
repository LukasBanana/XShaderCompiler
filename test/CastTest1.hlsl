
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
	SamplerState samplerState;
	float a;
	S1 b;
};

struct S3
{
	SamplerState samplerState;
};

void f1(S3 s3)
{
}

struct S4
{
};

float4 VS() : SV_Position
{
	//S2 a;
	//S2 a = (S2)0;
	
	S3 b;
	f1(b);
	
	S4 c;
	
	SamplerState samplerState;
	
	return (float4)1;
}
