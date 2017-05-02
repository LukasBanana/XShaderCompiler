
Buffer<float4> buf;
Texture2D tex;
Texture2DMS tex2;

float4 main() : SV_Position
{
	float4 val = buf[0u];
	float4 val2 = buf.Load(0u);
	float4 val3 = tex.Load(uint3(0, 0, 0));
	float4 val4 = tex2.Load(uint2(0, 0), 0);
	
	return val + val2 + val3 + val4;
}
