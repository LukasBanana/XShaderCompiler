
// RWTexture Test 2
// 21/03/2017

RWTexture2D<int2> tex;

void main()
{
	const int2 idx = int2(-3, 2);
	
	tex[idx];
	tex[idx] = 1;
	tex[idx] += 1;
}
