
// Scope Test 1
// 07/03/2017

struct main {};

struct VIn
{
    float3 pos : POSITION;
};

float fract(float x) { return frac(x)*2.0; }

SamplerState smpl;

void VS(VIn inp)
{
    main m;
    float3 pos = inp.pos * fract(1.5);
    
	int i = 1;
	for (int i = 2; i < 10; ++i)
	{
		int i = 3;
        int j = i*2;
		{
			int i = 4;
            int pos;
		}
	}
	
	SamplerState s;// = smpl;
    //s = smpl;
	
	for (SamplerState s; false; )
	{
		SamplerState s;
		//SamplerState s;
	}
}

