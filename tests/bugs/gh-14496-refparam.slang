// Discriminating pin for the field-address qualifier inheritance in
// `extractField`: the FIELD of a `__ref_readonly` parameter is the
// Generic-space shape the derivation actually changes — every Device-space
// pointer base was already covered by `maybePropagateAddressSpace` inside
// `emitFieldAddress`, but a ref-param type carries the Generic address
// space, so without the inheritance the field address of a `__ref_readonly`
// receiver loses its read-only qualifier and the guard would bind the
// `[mutating] ref` and write through the read-only receiver. With the
// inheritance, the guard refuses the ref: the write round-trips through the
// `[nonmutating] set` and the `set` call — not the `ref` call — appears in
// the IR.
//
// IR-level rather than executable: `__ref_readonly` is core-module-only
// (`-experimental-feature`), which the embedded compiler used by the CPU
// execution pipeline does not accept.

//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir -g0 -experimental-feature

RWStructuredBuffer<uint4> sideTable;

struct V
{
    uint4 data[4];

    __subscript(int i) -> uint4
    {
        get { return data[i]; }
        [nonmutating] set { sideTable[i] = newValue; }
        [mutating] ref { return data[i]; }
    }
};

struct Holder { V v; }

void throughReadonlyRef(__ref_readonly Holder h)
{
    h.v[0].x = 9;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Holder h0 = Holder();
    throughReadonlyRef(h0);
}

// The write must round-trip through the `set` accessor...
//IR: call %Vx5Foperatorx5Bx5Dx5Fset(
// ...and never bind the `[mutating] ref`.
//IR-NOT: call %Vx5Foperatorx5Bx5Dx5Fref(
