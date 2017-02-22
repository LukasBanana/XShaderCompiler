// Geometry Test 3
// 22/02/2017


// GEOMETRY SHADER

struct VSOutput
{
	float4 position : SV_Position;
	float4 color : COLORIN;
};

void ProcessVS(inout VSOutput o) {}

VSOutput VS(uint id : SV_VertexID)
{
    VSOutput o[2];
    
	ProcessVS(o[0]);
    
	// Set vertices
	o[0].position = 1;
	o[0].color = 2;
	
    o[1].position = 3;
	o[1].color = 4;
	
    if (id % 2 == 0)
        return o[0];
    else
        return o[1];
}


// GEOMETRY SHADER

struct GSOutput
{
	float4 position : SV_Position;
	float4 color : COLOR;
};

void ProcessGS1(inout GSOutput o) {}
void ProcessGS2(inout GSOutput o, VSOutput i) {}

[maxvertexcount(2)]
void GS(line VSOutput input[2], inout TriangleStream<GSOutput> stream)
{
	GSOutput vertex[2];
    
	// Process vertices
	ProcessGS2(vertex[0], input[0]);
	ProcessGS2(vertex[1], input[1]);
    
	// Listed expression test of "Append" intrinsic
    for (int i = 0; i < 1; stream.Append(vertex[0]), stream.Append(vertex[1]))
    {
        ++i;
    }
	
	// Expression statement of "Append" intrinsic
	stream.Append(vertex[0]);
	stream.Append(vertex[1]);
	stream.RestartStrip();
}
