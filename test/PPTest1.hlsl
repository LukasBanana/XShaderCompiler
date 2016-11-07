
// HLSL Translator: Preprocessor Test 1
// 05/11/2016

	#	include   "TestHeader1.h"   /*comment*/


// VERTEX SHADER

/*
#include ignore this include directive
*/

#define FOREVER for(;;)

   #    line 16 "TestShader3.hlsl"

// multi-line macro test
#define FOREVER_TEST1 \  
	for(;;) \  
	{ \ 
		int /*TEST_X*/ x=0; \
	}

// macro test with parameters
#define F1(X, Y) (X)*(Y)
#define  F2 (  X ,   Y  )  (X)*(Y)

#pragma
#pragma pack_matrix ()
#pragma def  

#ifdef _5
#ifdef F3
#	error F3 is defined
#else
#	define F3
/*#elif 1
#	error Wait what? ELIF
#else
#	error Wait what? ELSE*/
#endif
#endif

cbuffer Settings : register(b0)
{
	float4x4	wvpMatrix;
	float4x4	wMatrix;
	float4		diffuse;
};

struct VertexIn
{
	float3 position	: POSITION;/*
	Multi-line
	comment test
	*/
	float3 normal /*test*/	: NORMAL;
	float2 texCoord	: TEXCOORD0;
};

struct VertexOut
{
	float4 position	: SV_Position;
	float2 texCoord	: TEXCOORD0;
	float4 color	: COLOR;
};

//#error Foo F1(2) bar "Hello World"

VertexOut VS(VertexIn inp)
{
	VertexOut outp = (VertexOut)0;
	
	float pi2 = M_PI*2;
	
	FOREVER_TEST1
	
	FOREVER
	{
		
	}
	
	#include "TestHeader1.h"
	
	return outp;
}

#undef FOREVER
#undef FOREVER_TEST2




