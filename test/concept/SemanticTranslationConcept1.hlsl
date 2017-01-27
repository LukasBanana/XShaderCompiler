
// Example where the input and output structures can not be resolved,
// i.e. the structures must remain in the GLSL output

struct VIn
{
	float3 position : POSITION;
};

struct VOut
{
	float4 position : SV_Position;
};

void Transform(in VIn inp, out VOut outp)
{
	outp.position = float4(inp.position, 1);
}

VOut VS1(VIn inp)
{
	VOut outp;
	Transform(inp, outp);
	return outp;
}


