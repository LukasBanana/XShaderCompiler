
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

float4 VS() : SV_Position
{
	return float4(v0 + f1(), 1);
}


