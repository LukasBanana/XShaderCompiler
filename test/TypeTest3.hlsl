
// Type Test 3
// 13/12/2016

//BUG:
//  parser reads VarDeclStmnt;
//  type modifiers also not part of "VarType"
//  which is used for function declarations!
snorm float f1(const snorm float3 pos)
{
	return dot(pos, pos);
}

float4 VS() : SV_Position
{
	snorm float a = f1(0);
	unorm half4x4 b = 0;
	//snorm int c = 0; // ERROR: snorm/unorm only for floating-points
	
	
	return (float4)1;
}


