// Companion to 12386-ptr-to-empty-struct-compare.slang, for shader-slang/slang#12386.
//
// The arm that reports this is shared by all three type-legalization contexts, so the limit is not
// specific to empty structs: an interface-typed value is lowered away by existential legalization
// and reaches it the same way. This pins that second path so the diagnostic's wording stays honest
// about covering more than the empty-struct case.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag,non-exhaustive):-target cuda -entry computeMain -stage compute

interface IFoo
{
    int get();
}

struct Impl : IFoo
{
    int get() { return 1; }
}

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Impl i;
    IFoo boxed = i;
    Ptr<IFoo> p = __getAddress(boxed);
    output[0] = p == nullptr ? 1 : 0;
//diag:           ^^ operation is not supported on a value whose type has no representation on this target
}
