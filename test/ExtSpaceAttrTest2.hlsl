
// Language Extension Test 2 of "space" Attribute
// This is currenlty only a concept!
// 02/05/2017

cbuffer Matrices {
	// World-View-Projection Matrix: transforms from model to projection space
	[space(MODEL, PROJECTION)]
	float4 wvpMatrix;
	
	// View-Projection Matrix: transforms from world to projection space
	[space(WORLD, PROJECTION)]
	float4 vpMatrix;
	
	// Projection Matrix: transforms from view to projection space
	[space(VIEW, PROJECTION)]
	float4 pMatrix;
}

// Overloaded function: projects 'v' from model to projection space
[space(PROJECTION)]
float4 Project([space(MODEL)] float4 v) {
	return mul(wvpMatrix, v);
}

// Overloaded function: projects 'v' from world to projection space
[space(PROJECTION)]
float4 Project([space(WORLD)] float4 v) {
	return mul(vpMatrix, v);
}

// Overloaded function: projects 'v' from view to projection space
[space(PROJECTION)]
float4 Project([space(VIEW)] float4 v) {
	return mul(pMatrix, v);
}

// Overloaded function: returns 'v' in projection space
[space(PROJECTION)]
float4 Project([space(PROJECTION)] float4 v) {
	return v;
}

