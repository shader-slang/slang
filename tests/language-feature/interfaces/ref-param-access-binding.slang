// An argument is read unless its parameter is `out` or `__ref_writeonly`, so the memory location
// of a `__ref_writeonly` parameter may only be forwarded to those two modes. A `__ref_readonly`
// parameter does not denote a writable memory location, so it may not be forwarded to a mode
// that writes. A `__ref` parameter may be forwarded to any mode.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

void fIn(int v) {}
void fInOut(inout int v) {}
void fOut(out int v) { v = 0; }
void fRef(__ref int v) {}
void fRO(__ref_readonly int v) {}
void fWO(__ref_writeonly int v) { v = 1; }
void fCR(__constref int v) {}

void writeOnlyArgs(__ref_writeonly int x)
{
    fIn(x);
//CHECK:^ E30119
//CHECK:^ cannot read from writeonly, check modifiers.
    fInOut(x);
//CHECK:   ^ E30119
//CHECK:   ^ cannot read from writeonly, check modifiers.
    fOut(x);
    fRef(x);
//CHECK: ^ E30119
//CHECK: ^ cannot read from writeonly, check modifiers.
    fRO(x);
//CHECK:^ E30119
//CHECK:^ cannot read from writeonly, check modifiers.
    fWO(x);
    fCR(x);
//CHECK:^ E30119
//CHECK:^ cannot read from writeonly, check modifiers.
}

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
//CHECK:^ E30047
//CHECK:^ argument passed to parameter '0' must be l-value.
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

void writeOnlyReceivers(__ref_writeonly S s)
{
    s.byValue();
/*CHECK:
    ^ E30119
    ^ cannot read from writeonly, check modifiers.
*/
    s.mutate();
/*CHECK:
    ^ E30119
    ^ cannot read from writeonly, check modifiers.
*/
    s.byRef();
/*CHECK:
    ^ E30119
    ^ cannot read from writeonly, check modifiers.
*/
    s.byConstRef();
/*CHECK:
    ^ E30119
    ^ cannot read from writeonly, check modifiers.
*/
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

void atomics(__ref_writeonly int x, __ref_writeonly Atomic<int> a)
{
    InterlockedAdd(x, 1);
//CHECK:           ^ E30119
//CHECK:           ^ cannot read from writeonly, check modifiers.
    int orig;
    InterlockedAdd(x, 1, orig);
//CHECK:           ^ E30119
//CHECK:           ^ cannot read from writeonly, check modifiers.
    InterlockedCompareExchange(x, 0, 1, orig);
//CHECK:                       ^ E30119
//CHECK:                       ^ cannot read from writeonly, check modifiers.
    a.add(1);
/*CHECK:
    ^ E30119
    ^ cannot read from writeonly, check modifiers.
*/
    a += 1;
/*CHECK:
    ^ E30119
    ^ cannot read from writeonly, check modifiers.
*/
    a.store(1);
/*CHECK:
    ^ E30119
    ^ cannot read from writeonly, check modifiers.
*/
    a = 1;
}

void addresses(__ref_writeonly int x)
{
    let p = &x;
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
    let q = __getAddress(x);
//CHECK:                 ^ E30119
//CHECK:                 ^ cannot read from writeonly, check modifiers.
}

[numthreads(1, 1, 1)]
void main()
{
}
