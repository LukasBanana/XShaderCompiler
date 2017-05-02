
// RWTexture Test 4
// 01/05/2017

// ERROR: typed UAV loads are only allowed for single-component 32-bit element types 
RWTexture2D<int2> tex[2][3];

void main()
{
	int2 idx = int2(1, 2);
	
	// Different equivalent versions of "tex[int2(0, 0)].x"
	int a = tex[0][1][idx].x;
	int b = tex[0][1][idx][0];
	int c = (tex[0])[1][idx][0];
	int d = (tex[0][1])[idx][0];
	int e = (tex[0][1][idx])[0];
}
