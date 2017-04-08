
// Array Test 3
// 08/04/2017

uniform float index1D;
uniform float2 index2D;
uniform float array1D[10];
uniform float array2D[10][5];

float main() : COLOR
{
	float c = 0;
	
	c += array1D[index1D];
	c += array2D[index2D.x][index2D.y];
	
	return c;
}

