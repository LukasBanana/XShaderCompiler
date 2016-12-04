
// I/O Semantic Test 1
// 23/11/2016

cbuffer Matrices : register(c0)
{
	float4x4 wvpMatrix;
	float4x4 wMatrix;
};

struct VIn
{
	float4 vPos : POSITION;
	float3 vNormal : NORMAL;
	uint id : SV_VertexID;
};

struct VOut
{
	float4 tPos : SV_Position;
	float3 tNormal : NORMAL;
};

// VS1

float4 VS1(float4 vPos : POSITION, uint id : SV_VertexID) : SV_Position
{
	if (id == 0)
		return (float4)0;
	return mul(wvpMatrix, vPos);
}

// VS2

void VS2(in float4 vPos : POSITION, in uint id : SV_VertexID, out float4 tPos : SV_Position)
{
	tPos = mul(wvpMatrix, vPos);
}

// VS3

VOut VS3(VIn inp)
{
	VOut outp = (VOut)0;
	outp.tPos = mul(wvpMatrix, inp.vPos);
	outp.tNormal = inp.vNormal;
	return outp;
}

// VS4

void VS4(in VIn inp, out VOut outp)
{
	outp.tPos = mul(wvpMatrix, inp.vPos);
	outp.tNormal = mul(wMatrix, float4(inp.vNormal, 0)).xyz;
}

// VS5

float4 VS5(in VIn inp, float3 vTangent : TANGENT, out float3 tNormal : NORMAL) : SV_Position
{
	tNormal = inp.vNormal + vTangent;
	return mul(wvpMatrix, inp.vPos);
}

// VS6

float4 VS6(in VIn inp, uniform float4 offset) : SV_Position
{
	return mul(wvpMatrix, inp.vPos + offset);
}

// PS1

float4 PS1(in VOut inp) : SV_Target
{
	return (float4)saturate(dot(float3(0, 0, 1), inp.tNormal));
}


