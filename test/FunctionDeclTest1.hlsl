
// Function Declaration Test 1
// 30/11/2016


void f1()
{
	uint a = 0;
	int b = 0;
	uint d = 0u;
	int c = 0u;
}

uint f2()
{
	return 1.5;
	// ERROR -> no return statement
}

uint3 f3(float x)
{
	int3 v;
	if (x > 20)
		return v;
//	return v*2;
}

float4 VS() : SV_Position
{
	f1();
	f2();
	f3(0);
	return (float4)1;
}

