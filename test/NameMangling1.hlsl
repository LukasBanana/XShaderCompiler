
// HLSL Translator: Name Mangling Test 1
// 15/11/2016

float fract(float x)
{
	return x*2;
}

int main()
{
	return (int)fract(0);
}

[numthreads(1,1,1)]
void CS()
{
	int f = main();
}



