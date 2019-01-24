
// Constant Expression Test
// 12/12/2016

uniform float u0 = 1;

static const float c0 = 1;

float4 VS(uniform float u1) : SV_Position
{
	const float c1 = 1;
	
	#if 1
	
	c0 *= 5; // error
	
	c1 += 5; // error (unfortunately ignored by fxc)
	
	u0 = u0 + 5; // error
	
	u1 = u1 + 5; // error
	
	#endif
	
	struct Data
	{
		float foo;
		const float bar;
	};
	
	const Data data = { 0, 0 };
	data.foo = 0; // error
	
	return float4(c0, c1, u0, u1);
}
