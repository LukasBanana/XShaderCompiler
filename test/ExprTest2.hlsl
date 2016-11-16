
// HLSL Translator: Expression Test 2
// 14/11/2016

#define TEST 3

#if TEST == 1

void f(int x) {}
void f(int x, float y = 0.0) {}
void f(float x) {}

struct S1
{
	float x, y, z;
};

typedef struct S2
{
	int a, b;
}
S2_t[1][2][3];

#elif TEST == 2

void f(float x) { return x; }
void f(float3 x) {}
void g(int x) { return x; }

#elif TEST == 3

void f(float x) {}
void f(int x) {}

#endif

void CS()
{
	#if TEST == 1
	
	S1 s1 = (struct S1)0;
	
	S2_t s2;
	
	int i1 = (int).0f;
	
	float f1 = ((float3)1).x;
	
	float f2 = ((vector<float, (1+4)/5+3>)1).w;
	
	float f3 = f(i1);
	float f3 = f(1, 0);
	
	#elif TEST == 2
	
	int i1 = g(0);
	float f1 = f( ((int3)i1).x );
	//f(f);
	
	#elif TEST == 3
	
	struct S
	{
		float foo, bar;
	};
	
	S s;
	
	float a = s.foo.xx;
	int b = ((int3)0).x;
	
	//f(a.xxx);
	f(b);
	
	float4 pos = mul((float3x3)0, (float3)1);
	
	//float4 pos = mul((mul(1, 2)).x, (float3)1);
	
	#endif
	
}

