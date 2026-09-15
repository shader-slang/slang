// ref-accessor-immutable-base.slang
//
// Counterpart to nonmutating-ref-accessor.slang. Without `[nonmutating]`, a `ref`
// accessor is implicitly mutating, so a write through it requires a mutable base;
// on an immutable `let` the write is rejected, whereas the `[nonmutating]` form is
// allowed.
// https://github.com/shader-slang/slang/issues/9636, #12487

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static int gBacking = 0;

struct Proxy
{
    property int value
    {
        get { return gBacking; }
        ref { return gBacking; }
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    let p = Proxy();
    p.value = 7;
//CHECK:    ^ left of '=' is not an l-value
//CHECK:    ^ left of '=' is not an l-value.
}
