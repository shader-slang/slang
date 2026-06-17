//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// When a single generic type parameter must unify against two argument types
// that have no common supertype, constraint solving fails with no candidate
// type for the parameter. This previously produced the generic fallback; it
// should now report a focused unification conflict naming the parameter and
// both conflicting candidate types.

struct A
{
}
struct B
{
}

void foo<T>(T a, T b)
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    A a;
    B b;
    foo(a, b);
/*CHECK: cannot deduce generic argument for 'T': no common type for candidate types 'A' and 'B'
*/
}
