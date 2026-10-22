//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

// Test the potential-aliasing warning for reference, borrow, `out`, and `inout` parameters. Each
// parameter is named by the keyword of its passing mode, so a read-only or write-only reference
// is not reported as a plain `ref`. Every mode except `in` may refer to the argument's storage,
// because the compiler may pass `out`, `inout`, and borrow parameters by reference. Two such
// parameters warn if at least one may write the storage, unless both are reference parameters,
// whose function has opted into reference semantics. An `in` parameter never warns.

void readOnlyAndRef(__ref_readonly int a, __ref int b)
{
    b = a;
}

void writeOnlyAndReadOnly(__ref_writeonly int a, __ref_readonly int b)
{
    a = b;
}

void writeOnlyAndWriteOnly(__ref_writeonly int a, __ref_writeonly int b)
{
    a = 1;
    b = 2;
}

void refAndRef(__ref int a, __ref int b)
{
    a = b;
}

void borrowAndRef(__borrow int a, __ref int b)
{
    b = a;
}

void readOnlyAndInout(__ref_readonly int a, inout int b)
{
    b = a;
}

void readOnlyAndOut(__ref_readonly int a, out int b)
{
    b = a;
}

void refAndInout(__ref int a, inout int b)
{
    a = b;
}

void writeOnlyAndOut(__ref_writeonly int a, out int b)
{
    a = 1;
    b = 2;
}

void borrowAndInout(__borrow int a, inout int b)
{
    b = a;
}

void borrowAndOut(__borrow int a, out int b)
{
    b = a;
}

int readOnlyAndReadOnly(__ref_readonly int a, __ref_readonly int b)
{
    return a + b;
}

int borrowAndReadOnly(__borrow int a, __ref_readonly int b)
{
    return a + b;
}

int readOnlyAndIn(__ref_readonly int a, int b)
{
    return a + b;
}

void writeOnlyAndIn(__ref_writeonly int a, int b)
{
    a = b;
}

int borrowAndIn(__borrow int a, int b)
{
    return a + b;
}

void refAndIn(__ref int a, int b)
{
    a = b;
}

void inAndOut(int a, out int b)
{
    b = a;
}

int borrowAndConstRef(__borrow int a, __constref int b)
{
    return a + b;
}

int constRefAndReadOnly(const __ref int a, __ref_readonly int b)
{
    return a + b;
}

void test()
{
    int x = 0;

    borrowAndRef(x, x);
//CHECK:            ^ potentially aliased argument to a parameter that may be written
//CHECK:            ^ argument for 'ref' parameter 'b' may alias argument for 'borrow' parameter 'a'

    readOnlyAndInout(x, x);
//CHECK:                ^ potentially aliased argument to a parameter that may be written
//CHECK:                ^ argument for 'inout' parameter 'b' may alias argument for 'ref_readonly' parameter 'a'

    readOnlyAndOut(x, x);
//CHECK:              ^ potentially aliased argument to a parameter that may be written
//CHECK:              ^ argument for 'out' parameter 'b' may alias argument for 'ref_readonly' parameter 'a'

    refAndInout(x, x);
//CHECK:        ^ potentially aliased argument to a parameter that may be written
//CHECK:        ^ argument for 'ref' parameter 'a' may alias argument for 'inout' parameter 'b'

    writeOnlyAndOut(x, x);
//CHECK:            ^ potentially aliased argument to a parameter that may be written
//CHECK:            ^ argument for 'ref_writeonly' parameter 'a' may alias argument for 'out' parameter 'b'

    borrowAndInout(x, x);
//CHECK:              ^ potentially aliased argument to a parameter that may be written
//CHECK:              ^ argument for 'inout' parameter 'b' may alias argument for 'borrow' parameter 'a'

    borrowAndOut(x, x);
//CHECK:            ^ potentially aliased argument to a parameter that may be written
//CHECK:            ^ argument for 'out' parameter 'b' may alias argument for 'borrow' parameter 'a'

    readOnlyAndRef(x, x);
    writeOnlyAndReadOnly(x, x);
    writeOnlyAndWriteOnly(x, x);
    refAndRef(x, x);
    readOnlyAndReadOnly(x, x);
    borrowAndReadOnly(x, x);
    readOnlyAndIn(x, x);
    writeOnlyAndIn(x, x);
    borrowAndIn(x, x);
    refAndIn(x, x);
    inAndOut(x, x);
    borrowAndConstRef(x, x);
    constRefAndReadOnly(x, x);
}
