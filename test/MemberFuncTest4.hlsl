
// Member Function Test 3
// 15/03/2017

struct Base {
	int getA() {
		return a;
	}
	void setA(int x) {
		a = x;
	}
	int a, b;
};

// Should translate to something like this:
// "struct Sub { Base xsn_base; int a; };"
struct Sub : Base {
	int a, c;
};

float4 main() : COLOR {
	Sub s;
	
	//s.A::a = 1;
	s.setA(1);
	s.b = 2;
	
	s.a = 3;
	s.c = 4;
	
	return float4(s.getA(), s.b, s.a, s.c);
}

