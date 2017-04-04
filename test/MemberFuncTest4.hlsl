
// Member Function Test 3
// 15/03/2017

struct A {
	int get() {
		return a;
	}
	int a;
};

// Should translate to something like this:
// "struct S { A xsn_base; int a; };"
struct S : A {
	int a;
};

float4 main() : COLOR {
	S s;
	s.a = 1;
	s.A::a = 2;
	return float4(s.a, s.get(), 0, 0);
}

