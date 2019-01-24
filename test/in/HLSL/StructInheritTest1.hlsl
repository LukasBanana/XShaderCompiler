
// Structure Inheritance Test 1
// 13/11/2016

#if 1

struct S1 {
	float x;
};

struct S11 : S1 {
	float y;
};

struct S12 : S1 {
	float z;
};

struct S2 : S11 {
	float w;
};

float4 main() : COLOR
{
	struct Foo : S11
	{
		float w;
	}
	s_anonym;
	
	S2 s = (S2)4;
	return float4(s.x, s.y, 0, s.w);
}

#else

struct S : S {};

#endif
