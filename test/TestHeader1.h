
// Include Header Test 1
// 05/11/2016

#pragma once

/*THIS COMMENT MUST ONLY BE VISIBLE ONCE*/
void foo() { int bar=0; }

#pragma unknown_pramga /*hello world*/

#define M_PI 3.141592654  /*TEST_PI*/   

// redefinition test
#define M_PI /*FOO*/ 3.141592654

// redefinition test2
//#define M_PI 3.14159265  /*TEST_PI*/   


