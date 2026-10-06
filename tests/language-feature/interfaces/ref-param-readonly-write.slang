// The callee cannot write through a `readonly __ref` parameter, as with `const __ref`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry main -stage compute

void writeReadOnly(readonly __ref uint a[4])
{
    a[0] = 1;
//CHECK: E30011
}

void writeConst(const __ref uint a[4])
{
    a[0] = 1;
//CHECK: E30011
}

[numthreads(1, 1, 1)]
void main()
{
}
