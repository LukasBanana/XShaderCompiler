
// Structure Inheritance Test 3
// 05/04/2017

struct A {
	int a;
	static int static_a;
	int get1() { return a; }
};

struct B : A {
	int a;
	int get2() { return a; }
	int get3() { return A::a; }        // Access to local member of base "A"
	int get4() { return A::static_a; } // Access to static member of base "A"
};

struct C : B {
	int a;
	int get5() { return A::a; }
	int get6() { return B::a; }
};

int A::static_a = 1;

void main() {
	C s;
	s.a = A::static_a;
	s.C::a = 1;//A::a; //WRONG
	s.B::a = 2;
	s.A::a = 3;
	//s.A::get1();
	
	s.get1();
	s.get2();
	s.get3();
	s.get4();
	s.get5();
	s.get6();
}
