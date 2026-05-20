//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
struct R { int a; int b; int c; }
void main()
{
    R r = {1, 2, 3, 4, 5};
//CHECK:        ^ too many initializers
}
