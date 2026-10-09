// Reading through a `__ref_writeonly` parameter is an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

int readInInitializer(__ref_writeonly int x)
{
    int y = x;
//CHECK:    ^ E30119
//CHECK:    ^ cannot read from writeonly, check modifiers.
    return y;
}

int readInAssignment(__ref_writeonly int x)
{
    int y;
    y = x;
//CHECK:^ E30119
//CHECK:^ cannot read from writeonly, check modifiers.
    return y;
}

[numthreads(1, 1, 1)]
void main()
{
}
