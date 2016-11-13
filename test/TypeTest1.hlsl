
// HLSL Translator: Type Test 1
// 13/11/2016


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
S1_t1, S1_t1_a, S1_t1_b;

typedef struct
{
	float x, y;
}
S1_t2;

typedef struct S1 S1_t3;

typedef S1 S1_t4;

/* --- </typedef struct tests> --- */

void f2(Texture2D t)
{
	return 0;
}

float4 VS() : SV_Position
{
	DWORD x = 0;
	return float4(v0 + f1(), 1);
}


