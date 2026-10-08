//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct Test {
    RWStructuredBuffer<int> val;
    __subscript(int x, int y)->int
    {
        get { return val[x * 3 + y]; }
        set { val[x * 3 + y] = newValue; }
    }
}
Test test;

[numthreads(1, 1, 1)]
void computeMain()
{
    // `test` is a read-only parameter, so the subscript's mutating `set` accessor cannot use it as
    // a receiver. Diagnose that selected accessor rather than only classifying the expression as a
    // non-l-value.

    test[0,0] = 1;
//CHECK:      ^ mutating storage accessor cannot be called on an immutable value
//CHECK:      ^ mutating 'set' accessor cannot be called on an immutable value

}
