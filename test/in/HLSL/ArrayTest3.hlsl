
// Array Test 3
// 08/04/2017

uniform float index1D;
uniform float2 index2D;
uniform float array1D[10];
uniform float array2D[10][5];

float f() { return 1; }

float main() : COLOR
{
	float c = 0;
    
	c += array1D[1.0f];
	c += array1D[f()];
	c += array1D[index1D];
	c += array1D[sin(1)];
	c += array1D[sin(1.0f)];
	c += array2D[index2D.x][log10(1)];
	
	return c;
}

