// Layout attribute extension test
// 02/05/2017

[layout(rg8i)]
RWTexture2D<int2> tex;

[layout(rgba32f)]
RWTexture2D<float4> tex2;

RWTexture2D<float4> tex3;

void main()
{
	int a = tex[0].x;
	float b = tex2[0].x;
    float c = tex3[0].x;
}
