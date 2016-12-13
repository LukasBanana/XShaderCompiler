
// Type Test 3
// 13/12/2016

snorm float f1()
{
	return 0;
}

float4 VS() : SV_Position
{
	snorm float a = f1();
	unorm half4x4 b = 0;
	snorm int c = 0;
	
	
	return (float4)1;
}


