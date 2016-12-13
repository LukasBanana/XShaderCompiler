
// Expression Test 5
// 13/12/2016


float4 VS() : SV_Position
{
	int a = 0;
	bool b = true;
	
	//a + 1;
	a > 0 ? 1 : 2;
	
	[branch]
	b ? 1 : 2;
	
	return (float4)1;
}
