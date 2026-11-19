// Diagnostic companion to gh-14496.slang: pins the checker-side l-value
// contract the lowering guard relies on. A subscript whose only setter-or-ref
// accessors require a mutable receiver is not an l-value on a base that is
// not itself an l-value (a `let` is not), so writes are rejected at semantic
// checking. Pointer-member bases are l-values regardless of the pointer's
// access qualifier, so those writes are caught at lowering instead — see
// gh-14496-diag-lowering.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry computeMain -target spirv

RWStructuredBuffer<uint4> sideTable;

struct MutSetRef
{
    uint4 data[4];

    __subscript(int i) -> uint4
    {
        [mutating] set { sideTable[i] = newValue; }
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

[numthreads(1, 1, 1)]
void computeMain()
{
    // Neither subscript is an l-value here: the only setter-or-ref accessors
    // require a writable receiver, and a `let` base is not writable.
    let m = MutSetRef();
    m[0].x = 1;
//CHECK:   ^ left of '=' is not an l-value
//CHECK:   ^ left of '=' is not an l-value.

    let r = MutRefOnly();
    r[0].x = 2;
//CHECK:   ^ left of '=' is not an l-value
//CHECK:   ^ left of '=' is not an l-value.
}
