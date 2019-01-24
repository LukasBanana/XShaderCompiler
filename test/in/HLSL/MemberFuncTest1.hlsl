
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
struct A
{
    float a;
    
    void no_op() {}
};

float f3(int x, int y)
{
    return 3;
}

struct S : A
{
    // Foo bar
    int b;
	//float a;
    //struct {} test;
    //typedef float FLOAT;
    
    float f3()
    {
        return 1;
    }
    
    float f3(float x = 0)
    {
        return 2 + x;
    }
    
	float f2(float pos)
	{
        A tmp;
        tmp.no_op();
		return 2 + a + f3(a) + pos;//x*2;
	}
};

// Foo bar
float4 VS() : SV_Position
{
    A a;
    
	S s = (S)0;
    
    //s();
    
    #if 1
	return s.f2(1);
    #else
    return s.a*2 + f1();
    #endif
}

