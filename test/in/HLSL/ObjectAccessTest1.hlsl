
// Object Access Test 1
// 16/03/2017

float4 VS() : SV_Position
{
	float x = 1;
	
	( x = 2 ) = 3;
	
	return (float4)x;
}
