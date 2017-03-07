
// Scope Test 1
// 07/03/2017

int i()
{
	return 0;
}

void VS()
{
	int i = 1;
	for (int i = 2; i < 10; ++i)
	{
		int i = 3;
		{
			int i = 4;
		}
	}
	
	#if 0
	SamplerState s;
	for (SamplerState s; false; )
	{
		SamplerState s;
		//SamplerState s;
	}
	#endif
}

