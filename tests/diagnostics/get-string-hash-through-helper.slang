//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

// Passing a literal through a helper is legal: `performTypeInlining` inlines
// `h` into `main` so the surviving `getStringHash` operand is the literal
// "aaa", which folds to a constant hash.
//
// This is a regression guard for the string-hash check. Inlining leaves the
// now-unreferenced body of `h` behind, and inside it the `getStringHash` still
// reads the unfolded parameter, so a check that ran before dead code was
// eliminated would reject this shader on the strength of a function about to be
// deleted. `eliminateDeadCode` therefore runs before the check, not after it.

RWStructuredBuffer<uint> outputBuffer;

uint h(String s)
{
    return getStringHash(s);
}

[numthreads(1, 1, 1)]
void main()
{
    // CHECK: 807729185
    outputBuffer[0] = h("aaa");
}
