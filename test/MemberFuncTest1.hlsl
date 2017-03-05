
// Member Function Test 1
// 05/03/2017

cbuffer Settings
{
    typedef float4 F;
    float4x4 wvpMatrix;
    float4 offset;
    F f1()
    {
        return mul(wvpMatrix, offset);
    }
};

// Foo bar
struct A { float a; };

struct S : A
{
    // Foo bar
	float a;
	
    struct {} test;
    
    //typedef float FLOAT;
	float f2()
	{
		return 2;//x*2;
	}
};

// Foo bar
float4 VS() : SV_Position
{
    A a;
    
	S s;
	s.a = 1;
    
    #if 0
	return s.f();
    #else
    return s.a*2 + f1();
    #endif
}

