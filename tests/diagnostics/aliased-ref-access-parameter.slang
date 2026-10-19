//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

// Test that the potential-aliasing warning names each parameter by the keyword of its passing
// mode, so that a read-only or write-only reference is not reported as a plain `ref`.

void readOnlyAndRef(__ref_readonly int a, __ref int b)
{
    b = a;
}

void writeOnlyAndIn(__ref_writeonly int a, int b)
{
    a = b;
}

void test()
{
    int x = 0;

    readOnlyAndRef(x, x);
//CHECK:           ^ potentially aliased argument to 'out'/'inout'/'ref' parameter
//CHECK:           ^ argument for 'ref_readonly' parameter 'a' may alias argument for 'ref' parameter 'b'

    writeOnlyAndIn(x, x);
//CHECK:           ^ potentially aliased argument to 'out'/'inout'/'ref' parameter
//CHECK:           ^ argument for 'ref_writeonly' parameter 'a' may alias argument for 'in' parameter 'b'
}
