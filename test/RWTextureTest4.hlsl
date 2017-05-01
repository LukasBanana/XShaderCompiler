
// RWTexture Test 4
// 01/05/2017

// ERROR: typed UAV loads are only allowed for single-component 32-bit element types 
RWTexture2D<int2> tex;

void main()
{
	int2 idx = int2(1, 2);
	
	// Different equivalent versions of "tex[int2(0, 0)].x"
	int a = tex[idx].x;
	int b = tex[idx][0];
	int c = (tex)[idx][0];
	int d = (tex[idx])[0];
}
