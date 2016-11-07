
// HLSL Translator: Preprocessor Test 2
// 07/11/2016


#define F1(X, Y) (X)*(Y)
#define F2(X, Y) X*Y


F1 ( 1 , 2 )
F1 (1,2+3)
F2 (1,2+3)

#ifdef _0
F1(3,4,5)
F1 ( 6 )
F1
#endif

