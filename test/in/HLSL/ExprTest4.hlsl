
// Expression Test 4
// 27/11/2016


float4 VS() : SV_Position
{
	uint a = 1;
	int b = 2;
	uint c = a - b;
	uint d = b - a;
	
	b = a;
	a = b;
	
	return (float4)1;
}
