
// RWTexture Test 2
// 21/03/2017

RWTexture2D<int2> tex;

void main()
{
	int2 x;
	const int2 idx = int2(-3, 2);
	
	tex[idx];
	tex[idx] = 1;
	tex[idx] += 1;
	
	InterlockedAdd(tex[idx], 1);
	//InterlockedAdd(tex, 1, x);
	InterlockedCompareExchange(tex[idx], 1, 2, x);
}
