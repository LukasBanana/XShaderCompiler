
// Formatting Test 1
// 17/12/2016

struct S0
{
	float x;
	struct { float y; } s1;
};

int f1(int x)
{
	return x*2;
}

float4 VS() : SV_Position
{
	float a = 0, b = 0;
	int c = f1((int)b);
	
	if (a == 0)
		int d = 0;
	else
		int e = 0;
	
	if (a == 0)
	{
		int d = 0;
	}
	else if (a != 0)
	{
		int e = 0;
	}
	else { int f = 0; }
	
	if (a > b)
	{
		for (int i = 0; i < 10; ++i)
		{
			c++;
		}
		while (a < 0.5)
		{
			a += 0.1;
		}
	}
	else
	{
		switch (c)
		{
			case 0:
			{
				return (float4)0;
			}
			break;
		}
	}
	
	return (float4)1;
}


