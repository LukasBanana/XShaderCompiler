// GLSL Compute Shader
// Generated from HLSL Shader "CS"
// Sat Jan  3 21:56:22 1970

#version 330

#extension GL_ARB_derivative_control : enable
#extension GL_ARB_shading_language_420pack : enable

int test(int x)
{
    return 0;
}

void test2(int x, inout const mat4 y)
{
}

layout(local_size_x = 10, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uvec3 threadID = gl_GlobalInvocationID;
    uint groupIndex = gl_LocalInvocationIndex;
    
    int _z = 0;
    float _x = 3 * float(-gl_GlobalInvocationID.x);
    int _y = int(_x) * 2 + 2 - int((_x + 0.5)) + int(float((_z))) + 9;
    float _a = 1, _b = 2 + (_a += 4);
    #if 1
    int _mask = 256 | _y;
    #endif
    5 + 2, ++_mask, _mask <<= 2;
    const int _cnst0 = 0;
    int _cnst1 = 1;
    mat4 _cnst2;
    for (int _i = 0; _i < 10; ++_i)
        for (int _y = 0; _y < 20; _y++, ++_mask)
        {
            if (_x > _y + 2)
                _i++, --_i, gl_GlobalInvocationID.x++;
            else if (!(_x == 2))
            {
                int _y;
                _i += 4, ++_i, _i *= 2;
                _y = 2, _y = 4;
            }
            else
            {
                int _z;
                _x = _y;
            }
        }
    while (test(_x))
        do
        {
            mat4 _mat0;
            test2(_y, _mat0);
            do
                vec4 _v = 0;
            while (_v.x < 10);
        }
        while (bool((_x)) == true);
    switch (int(_x), _mask)
    {
        case 1:
            {
                int _x = 5;
                ;
                ;
                ;
                ;
                {
                    ;
                    ;
                    ;
                }
            }
            break;
        case 2:
            break;
        default:
            break;
    }
}

