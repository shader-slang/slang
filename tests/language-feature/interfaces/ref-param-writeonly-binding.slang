// An argument is read unless its parameter is `out` or `__ref_writeonly`, so the memory location
// of a `__ref_writeonly` parameter may only be forwarded to those two modes, and a receiver,
// atomic operation, or address-of that reads it is reported.
//
// Disabled: reads of a `__ref_writeonly` parameter are not yet diagnosed. A follow-up change adds
// that check and re-enables this test.

//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute -experimental-feature

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
