
void foo(inout int v) {}

void bar(inout int v[2]) {}

void main()
{
    int x;
    const int y; // Error: constant must be initialized
    float z;
    int v1[1], v2[2];
    
    foo(x); // Ok
    foo(y); // Error: 'y' is l-value declared as constant
    foo(z); // Error: 'z' is l-value with different type
    foo(1); // Error: '1' is r-value
    bar(v1); // Error: array is too small
    bar(v2); // Ok
}
