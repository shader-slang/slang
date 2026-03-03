//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry main -stage compute
// Using non-exhaustive because there are multiple cascading errors

extern static const int size = 1;

struct V
{
    int c[];
//CHECK: unsized member must be last
    int b[size];
    int a[];
}

struct Q
//CHECK: function parameter cannot be unsized
{
    V v1;
//CHECK: unsized member must be last
    V v2;
}

[numthreads(1,1,1)]
void main()
{
    V v;
//CHECK: cannot instantiate unsized type
    int i = 2;
}
