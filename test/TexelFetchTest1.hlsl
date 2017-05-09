
Buffer<float4> buf;

Texture2D   tex;
Texture2DMS tex2;
Texture2D   tex3[2][3];

float4 main() : COLOR {
	float4 c = 0;

	c += buf[0u];
	c += buf.Load(0u);
	c += tex.Load(uint3(0, 0, 0));
	c += tex2.Load(uint2(0, 0), 0);

	// Access with Operator[]
	uint3 i = 0;
	uint2 j = 0;

	c   +=  tex.Load(i);
	c   +=  tex[j];
	c   +=  tex3[1] [2].Load(i);
	c   += (tex3[1])[2][j];
	c   +=  tex3[1] [2][j];
	c.r += (tex3[1] [2][j])[0]; // Single color component
	c.r +=  tex3[1] [2][j] [0]; // Single color component

	Texture2D tex4 = tex;
	Texture2D tex5 = tex3[1][2];

	return c;
}
