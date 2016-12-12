
// Array Test 1
// 12/12/2016


struct Data
{
	float a0[5];
};

float4 VS() : SV_Position
{
	float a1[10];
	
	a1[0] = 1;
	
	Data data;
	data.a0[0] = 1.0f;
	
	return float4(a1[0], a1[1], a1[2], a1[3]);
}
