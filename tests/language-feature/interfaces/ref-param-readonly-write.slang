// The callee cannot write through a `__ref_readonly` parameter, nor through its legacy
// spellings `const __ref` and `__ref const`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry main -stage compute

void writeReadOnly(__ref_readonly uint a[4])
{
    a[0] = 1;
//CHECK: E30011
}

void writeConst(const __ref uint a[4])
{
    a[0] = 1;
//CHECK: E30011
}

void writeRefConst(__ref const uint a[4])
{
    a[0] = 1;
//CHECK: E30011
}

[numthreads(1, 1, 1)]
void main()
{
}
