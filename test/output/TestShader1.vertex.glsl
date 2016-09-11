// GLSL Vertex Shader
// Generated from HLSL Shader "VS"
// Sat Jan  3 21:56:22 1970

#version 330

#extension GL_ARB_derivative_control : enable
#extension GL_ARB_shading_language_420pack : enable

struct ParamStruct
{
    int param;
};

layout(std140, binding = 0) uniform VertexParam
{
    mat4 wvpMatrix;
    flat vec3 normal[3][2], test3;
    
    struct dataStruct
    {
        vec2 v0, v1;
        ivec2 v2;
    }
    data[10];
    ParamStruct param0;
};

struct TestStruct
{
    mat4 mat;
};

struct VertexIn
{
    vec3 coord;
    vec3 normal;
    vec2 texCoord;
    vec4 color;
};
in vec3 coord;
in vec3 normal;
in vec2 texCoord;
in vec4 color;

out _IVertexOut
{
    vec2 texCoord;
    vec4 color;
}
_outp;

vec3 GammaCorrect(vec3 color, float gamma)
{
    return pow(color, 1.0 / gamma);
}

void main()
{
    VertexIn inp;
    inp.coord = coord;
    inp.normal = normal;
    inp.texCoord = texCoord;
    inp.color = color;
    uint vertexID = gl_VertexID;
    
    TestStruct _test = TestStruct(0);
    gl_Position = ((wvpMatrix) * (vec4(inp.coord, 1.0)));
    _outp.texCoord = inp.texCoord;
    vec3 _lightDir = vec3(0.5);
    vec3 _lightDir2 = { 0.5, -0.5, 1.0 };
    vec3 _normal = normalize(inp.normal);
    if (gl_VertexID < 3)
    {
        float _NdotL = dot(_normal, -normalize(_lightDir));
        float _shading = max(0.2, _NdotL);
    }
    _outp.color = vec4(GammaCorrect(inp.color.xyz, 1.2), 1.0);
    {
        return;
    }
}

