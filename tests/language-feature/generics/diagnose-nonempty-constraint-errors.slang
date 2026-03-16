//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// Test: various invalid uses of the nonempty(...) constraint
// produce errors 30413, 30414, 30415, and 30416.

// Error 30413: nonempty applied to a non-pack type.
struct BadNonemptyOnType<each T> where nonempty(int) {}
//CHECK: expected a direct reference to a generic pack parameter here

// Error 30415: optional nonempty is not meaningful.
int badOptionalNonempty<let each D : int>() where optional nonempty(D)
//CHECK: remove `optional` from this `nonempty(...)` constraint
{
    return 0;
}

// Error 30416: nonempty on a pack from an outer generic.
struct Outer<let each D : int>
{
    struct Inner<let each E : int> where nonempty(D) {}
//CHECK: this pack parameter is declared outside the current generic
}

// Error 30412: forwarding an unconstrained pack to a nonempty-constrained parameter.
void consumeNonEmpty<let each D : int>() where nonempty(D) {}

void badForwardUnconstrained<let each D : int>()
{
    consumeNonEmpty<D>();
//CHECK: nonempty(...) requires a non-empty pack
}
