// Geometry Test 4
// 22/02/2017

struct VSOutput
{
	float4 position : SV_Position;
};

struct GSOutput
{
	float4 position : SV_Position;
};

[maxvertexcount(2)]
void GS(line VSOutput inp[2], inout PointStream<GSOutput> stream)
{
    // Other test
    GSOutput a, b;
    a.position = 1;
    b.position = 2;
    stream.Append(b);
    stream.Append(a);
}
