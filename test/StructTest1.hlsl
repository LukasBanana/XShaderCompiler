
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

S2_t f1()
{
	return (S2_t)0;
}

struct { float value; } f2()
{
	struct { float value2; } result = (struct { float value3; })0;
	result.value2 = 1;
	return result;
}

struct { float value4; } f3()
{
	return (struct { float value5; })0;
}

float4 main() : SV_Position
{
	S1 s1;
	float b = s1.s1_2.b;
	
	struct UnusedStruct
	{
	};
	
	struct
	{
		float hello, world;
	}
	s2;
	
	struct
	{
		float hello, world[2];
	}
	s3;
	
	s3.world[0] = 2;
	
	return (float4)(f1().x + f2().value + f3().value5);
}


