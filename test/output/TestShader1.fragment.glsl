// GLSL Fragment Shader
// Generated from HLSL Shader "PS"
// Sat Jan  3 21:56:22 1970

#version 330

#extension GL_ARB_derivative_control : enable
#extension GL_ARB_shading_language_420pack : enable

layout(origin_upper_left) in vec4 gl_FragCoord;

layout(location = 0) out vec4 SV_Target0;

layout(std140, binding = 1) uniform PixelParam
{
    vec4 ambientColor;
};

in _IVertexOut
{
    vec2 texCoord;
    vec4 color;
}
inp;

layout(binding = 0) uniform sampler2D tex;

void Frustum(inout vec4 v);

void Frustum(inout vec4 v);

void Frustum(inout vec4 v, int x)
{
}

void TexTest(sampler2D t2d)
{
}

void main()
{
    vec3 _interpColor = vec3(1.0, 0.0, 0.0);
    vec4 _diffuse = mix(vec4(1.0), clamp(texture(tex, inp.texCoord), 0.0, 1.0), inp.position.x);
    vec2 _tc_dx = dFdxCoarse(inp.texCoord);
    vec4 _viewRay = vec4(0.0);
    Frustum(_viewRay);
    TexTest(tex);
    int _intrl;
    atomicAdd(_intrl, 5);
    {
        SV_Target0 = ambientColor + clamp(inp.color * _diffuse, 0.0, 1.0);
        return;
    }
}

void Frustum(inout vec4 v)
{
    v.x = v.x * 0.5 + 0.5;
    v.y = v.y * 0.5 + 0.5;
}

void Frustum(inout vec4 v);

