
// Buffer Objects Test
// 10/12/2016

cbuffer Settings : register(b0)
{
	uint3 groupSize;
};

SamplerState smpl0[10];
Texture2D tex0[10];

Buffer<float4> inBuffer : register(t0);

RWBuffer<float4> outBuffer : register(u0);

struct Data
{
	float4 position;
	float4 color;
};

StructuredBuffer<Data> lightSources : register(t1);


[numthreads(1, 1, 5*9+(int)1.0)]
void CS(uint3 groupID : SV_GroupID)
{
//	float4 color = tex0.Sample(smpl0[0], (float2)0);
	
	// Get global index
	uint i = groupID.z*groupSize.x*groupSize.y + groupID.y*groupSize.x + groupID.x;
	
	// Structure buffer access test
	int4 pos = lightSources[i].position;
	
	Datas data = lightSources[i].position.x;
	
	// Flip components
	outBuffer[i] = inBuffer[i].wzxy;
}

