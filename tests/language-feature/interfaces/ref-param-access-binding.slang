// A `__ref_readonly` parameter denotes a memory location that is not writable, so it may be
// forwarded to a mode that only reads, including `__ref_readonly`, but not to one that writes.
// A `__ref` parameter may be forwarded to any mode.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute -experimental-feature

void fIn(int v) {}
void fInOut(inout int v) {}
void fOut(out int v) { v = 0; }
void fRef(__ref int v) {}
void fRO(__ref_readonly int v) {}
void fWO(__ref_writeonly int v) { v = 1; }
void fCR(__constref int v) {}

void readOnlyArgs(__ref_readonly int x)
{
    fIn(x);
    fInOut(x);
//CHECK:   ^ E30047
//CHECK:   ^ argument passed to parameter '0' must be l-value.
    fOut(x);
//CHECK: ^ E30047
//CHECK: ^ argument passed to parameter '0' must be l-value.
    fRef(x);
//CHECK: ^ E30047
//CHECK: ^ argument passed to parameter '0' must be l-value.
    fRO(x);
    fWO(x);
//CHECK:^ E30047
//CHECK:^ argument passed to parameter '0' must be l-value.
    fCR(x);
}

void readWriteArgs(__ref int x)
{
    fIn(x);
    fInOut(x);
    fOut(x);
    fRef(x);
    fRO(x);
    fWO(x);
    fCR(x);
}

void localArgs()
{
    int x = 0;
    fIn(x);
    fInOut(x);
    fOut(x);
    fRef(x);
    fRO(x);
    fWO(x);
    fCR(x);
}

struct S
{
    int v;
    int byValue() { return v; }
    [mutating] void mutate() { v++; }
    [__ref] void byRef() { v = 0; }
    [constref] int byConstRef() { return v; }
}

void readOnlyReceivers(__ref_readonly S s)
{
    s.byValue();
    s.mutate();
//CHECK:    ^ E30050
//CHECK:    ^ mutating method 'mutate' cannot be called on an immutable value
    s.byRef();
//CHECK:   ^ E30050
//CHECK:   ^ mutating method 'byRef' cannot be called on an immutable value
    s.byConstRef();
}

void readWriteReceivers(__ref S s)
{
    s.byValue();
    s.mutate();
    s.byRef();
    s.byConstRef();
}

[numthreads(1, 1, 1)]
void main()
{
}
