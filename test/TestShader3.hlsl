
// HLSL Translator: Shader Test 3
// 05/11/2016

	#	include   "TestHeader1.h"   /*comment*/


// VERTEX SHADER

/*
#include ignore this include directive
*/

#define FOREVER for(;;)

// multi-line macro test
#define FOREVER_TEST1 \  
	for(;;) \  
	{ \ 
		int x=0; \
	}

// redefinition test
#define FOREVER_TEST1

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

#undef FOREVER_TEST1
#undef FOREVER_TEST2




