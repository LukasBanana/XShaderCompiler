
// HLSL Translator: Expression Test 2
// 14/11/2016

#define TEST 2

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
void g(int x) { return x; }

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
	
	struct {float x;} i1 = g(0);
	float f1 = f(i1);
	//f(f);
	
	#endif
	
}

