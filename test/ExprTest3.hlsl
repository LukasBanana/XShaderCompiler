
// Expression Test 2
// 22/11/2016


float4 VS() : SV_Position
{
	struct A { int z; };
	typedef A B;
	typedef int X;
	int Y = 0;
	(int) - (1);
//	((int)) - (1); // illegal
	(Y) - (1);
	(X) - (1);
//	((X)) - ((1)); // illegal
	((Y)) - ((1));
	(A) - ((1));
	(B) - ((1));
	(X)0;
}
