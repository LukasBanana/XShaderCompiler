// GLSL Fragment Shader
// Generated from HLSL Shader "PS"
// Sat Jan  3 21:56:22 1970

#version 330

#extension GL_ARB_shading_language_420pack : enable

layout(origin_upper_left) in vec4 gl_FragCoord;

layout(location = 0) out vec4 SV_Target0;

in _IVertexOut
{
    vec2 texCoord;
    vec4 color;
}
inp;

layout(binding = 0) uniform sampler2D tex;

void main()
{
    {
        SV_Target0 = inp.color * texture(tex, inp.texCoord);
        return;
    }
}

