// The callee cannot write through a `__ref_readonly` parameter, nor through its legacy
// aliases `const __ref` and `__ref const`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry main -stage compute -experimental-feature

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
