
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
	S2 s = (S2)0;
	return s.x;
}
