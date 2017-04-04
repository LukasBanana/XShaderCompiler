
// Structure Inheritance Test 2
// 04/04/2017

struct A { int a; };

struct B : A {};

struct C : B {};

void main() {
	C c;
	c.a = 1;
	B b;
	b.a = 2;
	A a;
	a.a = 3;
}

