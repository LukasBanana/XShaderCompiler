
// Structure Inheritance Test 3
// 05/04/2017

struct A {
	int a;
	int get1() { return a; }
};

struct B : A {
	int a;
	int get2() { return a; }
};

struct C : B {
    int a;
};

float4 main() : COLOR {
	C s;
	s.a = 1;
	s.C::a = 1;
	s.B::a = 2;
	s.A::a = 3;
	//s.A::get1();
	return float2(s.get1(), s.get2()).xyxy;
}
