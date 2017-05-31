
// Member Function Test 5
// 30/05/2017

struct A {
	int x_;
	
	void f1() {
		int x = x_; // x = self.x
	}
};

struct B : A {
	int y_;

	void f2() {
		f1(); // f1(self.base)
		
		A a;
		a.f1(); // f1(a)
		
		B b;
		b.f1(); // f1(b.base)
		
		int x = x_; // x = self.base.x
		int y = y_; // y = self.y
	}
};

void main() {
	A a;
	a.f1(); // f1(a)
	
	B b;
	b.f1(); // f1(b.base)
	b.f2(); // f2(b)
	
	int x = b.x_; // x = b.base.x
	int y = b.y_; // y = b.y
}
