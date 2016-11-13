
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

float4 PS() : SV_Target
{
	struct Foo : S11
	{
		float w;
	}
	s_anonym;
	
	S2 s = (S2)4;
	return float4(s.x, s.y, 0, s.w);
}
