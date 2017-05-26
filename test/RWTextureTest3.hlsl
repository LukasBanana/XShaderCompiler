
RWTexture2D<int> tex[2][3];

void main()
{
	#if 0//TEST for error handling
	uint2 x;
	#else
	uint x;
	#endif
	const uint2 idx = uint2(-3, 2);
	
	InterlockedAdd(tex[1][2][idx], 1u);
	InterlockedCompareExchange((tex[1])[2][idx], 1u, 2u, x);
}
