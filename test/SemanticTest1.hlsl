
// I/O Semantic Test 1
// 23/11/2016

cbuffer Matrices
{
	float4x4 wvpMatrix;
};

struct VIn
{
	float4 vPos : POSITION;
};

struct VOut
{
	float4 tPos : SV_Position;
};

// VS1

float4 VS1(float4 vPos : POSITION) : SV_Position
{
	return mul(wvpMatrix, vPos);
}

// VS2

void VS2(in float4 vPos : POSITION, out float4 tPos : SV_Position)
{
	tPos = mul(wvpMatrix, vPos);
}

// VS3

VOut VS3(VIn inp)
{
	VOut outp = (VOut)0;
	outp.tPos = mul(wvpMatrix, inp.vPos);
	return outp;
}

// VS4

void VS4(in VIn inp, out VOut outp)
{
	outp.tPos = mul(wvpMatrix, inp.vPos);
}

// VS5

float4 VS5(in VIn inp, float3 vNormal : NORMAL, out float3 tNormal : NORMAL) : SV_Position
{
	tNormal = vNormal;
	return mul(wvpMatrix, inp.vPos);
}


