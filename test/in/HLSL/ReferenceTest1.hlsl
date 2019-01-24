
// Reference Test 1
// 01/03/2017

struct PIn
{
    float2 tc : TEXCOORD;
};

struct POut
{
    float4 color : SV_Target;
};

Texture2D tex;
SamplerState smpl;

// NOTE:
//   This function will force the compiler
//   to keep the "POut" struct, although this
//   function will be removed in the output
SamplerState T(SamplerState s, POut o)
{
    return s;
}

POut PS(PIn i)
{
    POut o;
    o.color = tex.Sample(T(smpl, o), i.tc);
    return o;
}

