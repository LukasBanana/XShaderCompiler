
// HLSL Translator: Expression Test 2
// 14/11/2016

#define TEST 1

#if TEST == 1

Texture2D tex;

float f(int x) {}
float f(inout int x, float y = 0.0)
{
	return sin(y);
	SamplerState samp;
	return tex.Sample(samp, (float2)y);
}

void f(float x) {}

struct S1
{
	float x <int annotation1 = 0, y=0; string str="hello annotations";>, y, z;
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

static const int g_const1 <int annotation1 = 0, y=0; string str="hello annotations";>;

#endif

void CS() <int annotation=0; string info="hello world!";>
{
	return acos();
	
	#if TEST == 1
	
	struct LocalStruct
	{
		float x,y;
		S2 s2;
	};
	
	LocalStruct local_s[2][1];
	int local_s_a = local_s[0][1].s2.b;
	
	S1 s1 = (struct S1)0;
	
	S2_t s2;
	
	s1 = s2[0][0][0].a;
	
	int i1 = (int).0f;
	
	float f1 = ((vector)1).xyz.x.xxx.zz.x.x.x;
	
	float f2 = ((vector<float, (1+4)/5+3>)1).w;
	
	#if 1
	float f3 = f(i1, 0);
	float f4 = f(1, 0);
	//#else
	f(0.0);
	#endif
	
	float f5 = ((matrix)0)._11_m10_11;
	float f6 = float3(1,2,3).z;
	
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
	
	//s = (struct{})g_const1;
	
	//float a = s.foo.xx * ( (S)g_const1 + {1,2,3}  ) + 1;
	
	//f(a.xxx);
	//f(b);
	
	//string x = "test";
	
	float4 pos1 = mul((float3x3)0, (float3)1);
	
	//float4 pos2 = mul((mul(1, 2)).x, (float3)1);
	
	#elif TEST == 4
	
	struct S { int s; };
	
	int a = (int)(float)(S)1 + 2;
	S   b = (S)1 + 2;
	S   c = (S) + + - + 1;
	S   d = (S) - 1;
	int e = (int)(float)(S)(1 + 2);
	//S   f = (struct {float x;})1;
	
	#endif
	
}

#if 0

technique T0 {
	pass P0 {
		PixelShader = compile ps_5_0 PS();
		
	}
}

#endif

