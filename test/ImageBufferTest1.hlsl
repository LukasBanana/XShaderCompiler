
// RWTexture Test 2
// 21/03/2017

RWBuffer<float4> gOutputBuf;
Buffer<float4> gInputBuf;

[numthreads(8, 8, 1)]
void main(
	uint3 dispatchThreadId : SV_DispatchThreadID,
	uint threadIndex : SV_GroupIndex)
{
	float4 value = gInputBuf[threadIndex];
	gOutputBuf[threadIndex] = value;
}
