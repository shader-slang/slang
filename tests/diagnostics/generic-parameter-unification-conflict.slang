//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// When a single generic parameter must unify against two conflicting arguments,
// constraint solving fails with no consistent value for the parameter. This
// previously produced the generic fallback; it should now report a focused
// unification conflict naming the parameter and both conflicting candidates,
// for both type parameters (no common type) and value parameters (two different
// required values).

struct A
{
}
struct B
{
}

void foo<T>(T a, T b)
{
}

void bar<let N : int>(int a[N], int b[N])
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    A a;
    B b;
    foo(a, b);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'A' and 'B'
*/

    int x[3];
    int y[4];
    bar(x, y);
/*CHECK: cannot deduce generic argument for 'N': conflicting requirements '3' and '4'
*/
}
