
// Example where the input and output structures can not be resolved,
// i.e. the structures must remain in the GLSL output

in vec3 position;

struct VIn
{
	vec3 position;
};

struct VOut
{
	vec4 position;
};

void Transform(in VIn inp, out VOut outp)
{
	outp.position = vec4(inp.position, 1);
}

void main()
{
	VOut outp;
	outp.position = position;
	Transform(inp, outp);
	gl_Position = outp.position;
}


