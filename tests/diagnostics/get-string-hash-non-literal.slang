//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

// `getStringHash` hashes the bytes of a string at compile time, so by the time
// the string-hash check runs its argument must have folded down to a string
// literal. Here the argument is a `static String` global, which lowers to a
// `load` of the global rather than to a literal, so there are no bytes to hash
// and the call must be rejected.
//
// This shape matters separately from the run-time-selected one: there is no
// run-time selection anywhere in this shader. The condition the check enforces
// is "operand 0 did not fold to a literal", which a plain global variable read
// satisfies just as a `?:` on a uniform does.

RWStructuredBuffer<uint> outputBuffer;

static String g = "aaa";

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = getStringHash(g);
//CHECK: E41023
//CHECK:                           ^ getStringHash can only be called when argument is statically resolvable to a string literal
}
