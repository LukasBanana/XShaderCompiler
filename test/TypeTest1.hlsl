
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

/*typedef struct S1
{
	float x, y;
}
S1_t;*/

float4 VS() : SV_Position
{
	DWORD x = 0;
	return float4(v0 + f1(), 1);
}


