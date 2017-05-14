
// Struct Test 2
// 14/05/2017

struct Base {
	int first;
};

struct UnusedStruct : Base {
    float foo;
};

struct { int x; float y; } f() {
	return (struct { int i; float f; })0;
}

struct { float2 value; } g() {
    struct {
        float2 bar;
    } x;
    x.bar = 0;
	return x;
}

float4 main() : COLOR {
	return g().value * f().x * f().y;
}



