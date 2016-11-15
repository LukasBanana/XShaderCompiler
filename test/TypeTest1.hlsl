
// HLSL Translator: Type Test 1
// 13/11/2016

struct S0
{
	float x;
};


cbuffer buffer1 : register(b0)
{
	vector<float, (4 > 2 ? (3+1)/4+3 : 2)> v0;
	matrix<double, 2, 3> m0;
};

matrix<float, 3, 3> f1()
{
	return (float3)0;
}

typedef int DWORD;
typedef float FLOAT; 
typedef vector <float, 4> VECTOR;
typedef matrix <float, 4, 4> MATRIX;
//typedef string STRING;
//typedef texture TEXTURE;
//typedef pixelshader PIXELSHADER;
//typedef vertexshader VERTEXSHADER;

/* --- <typedef struct tests> --- */

typedef struct S1
{
	float x, y;
}
S1_t1, S1_t1_a[5], S1_t1_b[1][2][3];

typedef struct
{
	float x, y;
}
S1_t2;

typedef struct S1 S1_t3;

typedef S1 S1_t4;

/* --- </typedef struct tests> --- */

//void f2(Texture2D t[5])
int f2(int x)
{
	return 0;
}

S1_t1_a f3()
{
	return (S1_t1_a)0;
}

const int g_2 = 5;
float g_1 = 0.0;

const struct { struct { float inner2; } inner1; } g_anomym_s1;

struct { float x; } f4()
{
	//return (struct { float x; })0;
}

float4 VS() : SV_Position
{
	struct S1
	{
		float f, g;
	}
	s1, s2, s3;

	float3 f2_v = f2(0).xxx;
	
	//float3 f3_v = f3()[0].xxx;
	//float3 f3_v = f3()[0].xxx;
	
	float f4_x = g_anomym_s1.inner1.inner2;
	
	int a = 0, b = 0;
	int c = (a += b);
	
	DWORD x = 0;
	float x1 = 0;
	S1_t2 x2 = 0;
	return float4(v0 + f1(), 1);
}


