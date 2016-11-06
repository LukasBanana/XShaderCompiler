
// HLSL Translator: Expression Test 1
// 06/11/2016

[numthreads(1, 2, 3)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
	int a = (int)threadID.x * 2 + 2 - (int)(threadID.x + 0.5) + (int)(float)(threadID.z) + 9;
	int b = (int)threadID.x * 2 + 2 - (int)(threadID.x + 0.5) + (int)(float)(a) - 9;
}

