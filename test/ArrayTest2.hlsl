
// Array Test 2
// 21/12/2016


int f1()
{
	return 1;
}

float4 V() : SV_Position
{
	/*const int c3 = 1.5f + f1();
	const int c2 = 2;//*c3;
	const int c1 = 4+c2;

	int a[c2+1] = { 1, 2, 3 };
	int b = a[0];*/

	typedef int ArrayType2[2];
	ArrayType2 d = { 1, 2 };
	//typedef ArrayType2 ArrayType2_2[2];
	//ArrayType2_2 e = { d, d };

	int c[][2][3] =
	{
		{ { 1, 2 }, /*{ 2, 3 }*/d },
		{ { 4, 5 }, { 6, 7 } }
	};

	return 1;
}

