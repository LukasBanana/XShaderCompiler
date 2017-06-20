
// Preprocessor Test 3 Header
// 20/06/2017

#ifndef N
#	define N 0
#endif

#define DECL_VECTOR4(X) uniform vec4 vector_##X

#define N N+1

DECL_VECTOR4(N)
DECL_VECTOR4(__EVAL__(N))

#if N < 10
#	include "PPTest3Header.vert"
#endif

