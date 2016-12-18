
// Struct Test 1
// 01/12/2016

struct S1
{
	struct
	{
		float a;
	}
	s1_1;
	
	struct
	{
		float b;
	}
	s1_2;
	
	struct
	{
		struct
		{
			float foo, bar;
		}
		s1_3_1;
	}
	s1_3;
};

typedef struct S2 { float x, y; } S2_t;

S2_t f2()
{
	return (S2_t)0;
}

struct { float value; } f1()
{
	return 0;
}

float4 VS() : SV_Position
{
	S1 s1;
	
	struct UnusedStruct
	{
	};
	
	struct
	{
		float hello, world;
	}
	s2;
	
	return (float4)(f1().value + f2().x);
}


