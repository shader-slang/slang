// Diagnostic companion to gh-14496.slang: pins the lowering-side refusals
// that make this fix safe for pointer-member bases (which are l-values
// regardless of the pointer's access qualifier, so the checker does not
// reject them). Reading through a getter-less subscript whose only `ref`
// accessor requires a mutable receiver is refused at lowering on a
// non-writable base (E40027), and taking the address of a guarded subscript
// that lowering cannot collapse into a `ref`-accessor call is refused
// (E40008). A `[nonmutating] ref` needs no writable receiver and still reads.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry computeMain -target spirv

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

struct MutRefOnly
{
    uint4 data[4];

    __subscript(int i) -> uint4
    {
        [mutating] ref { return data[i]; }
    }
};

struct NonMutRefOnly
{
    uint4 data[4];

    __subscript(int i) -> uint4
    {
        [nonmutating] ref { return data[i]; }
    }
};

struct Holder { V v; MutRefOnly m; NonMutRefOnly n; }

[numthreads(1, 1, 1)]
void computeMain(Ptr<Holder, Access.Read> p)
{
    // A `[nonmutating] ref` does not require writable storage, so the read is
    // allowed and must produce no diagnostic.
    uint4 ok = p->n[0];
    sideTable[0] = ok;

    // A `[mutating] ref` cannot be invoked on a read-only base.
    uint4 t = p->m[0];
//CHECK:  ^ cannot invoke this subscript's accessor: it requires a mutable receiver, but the base does not provide writable storage
//CHECK:  ^ the accessor's receiver may write through the base, and the base's storage does not permit writes
    sideTable[1] = t;

    // Address-taking is aggressive: it cannot fall back to a `set`
    // round-trip, so the guarded subscript is refused.
    uint4* a = &p->v[0];
//CHECK:            ^ the form of this l-value argument is not valid for a `ref` parameter
    sideTable[2] = *a;
}
