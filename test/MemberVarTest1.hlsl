
// Member Variable Test 1
// 15/03/2017

struct Light
{
	static float4 getColor()
	{
		return color;
	}

	// Can only be declared here
	static float4 color;
};

// Must be defined here
float4 Light::color = 1;

float4 PS() : SV_Target
{
	return Light::getColor();
}



