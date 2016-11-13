
// HLSL Translator: Type Test 1
// 13/11/2016


cbuffer buffer1 : register(b0)
{
	vector<float, 3> v0;
	matrix<int, 2, 3> m0;
};


float4 VS() : SV_Position
{
	return float4(v0, 1);
}


