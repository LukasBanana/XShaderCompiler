
// Initializer Test 1
// 13/03/2017


float4 main(uint id : SV_VertexID) : SV_Position
{
	// Should be converted to "vec2[](vec2(1, 2), vec2(3), vec2(4))" for a lower GLSL version than 420
    float2 posArray[] =
    {
        float2(1, 2), 3, 4
    };
    
    //(b) = 1;
    
    return posArray[id].xyxy;
}

