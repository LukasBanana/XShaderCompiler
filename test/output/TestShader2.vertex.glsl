// GLSL Vertex Shader
// Generated from HLSL Shader "VS"
// Sat Jan  3 21:56:22 1970

#version 330

#extension GL_ARB_shading_language_420pack : enable

layout(std140, binding = 0) uniform VertexParam
{
    mat4 wvpMatrix;
};

struct VertexIn
{
    vec3 coord;
    vec3 normal;
    vec2 texCoord;
    uint id;
};
in vec3 coord;
in vec3 normal;
in vec2 texCoord;
in uint id;

out _IVertexOut
{
    vec2 texCoord;
    vec4 color;
}
_outp;

void main()
{
    VertexIn inp;
    inp.coord = coord;
    inp.normal = normal;
    inp.texCoord = texCoord;
    inp.id = id;
    
    gl_Position = ((wvpMatrix) * (vec4(inp.coord, 1.0)));
    _outp.texCoord = inp.texCoord + (gl_Position.xy);
    vec3 _lightDir = { 0.5, -0.5, 1.0 };
    vec3 _normal = normalize(inp.normal);
    float _NdotL = dot(_normal, -normalize(_lightDir));
    float _shading = max(0.2, _NdotL);
    _outp.color = vec4(vec3(_shading), 1.0);
    {
        return;
    }
}

