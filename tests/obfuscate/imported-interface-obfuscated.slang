//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -obfuscate

// A cross-module interface reference compiled with `-obfuscate`, which takes the
// AST-derivation fallback in `tryBorrowInterfaceFromOwningModule` rather than the
// borrowing path.
//
// Worth being precise about what this pins. It covers the fallback end to end:
// the imported interface is still lowered completely and the requirement below
// still resolves through it. It does *not* pin the obfuscation guard itself --
// when the library is lowered by the same obfuscating request its symbols are
// hashed as well, so with the guard removed the symbol search finds nothing and
// falls through to the same place, and the emitted code is byte-identical.

import "imported-interface-obfuscated-lib";

RWStructuredBuffer<float> outputBuffer;

float useScaled<T : IScaledObf>(T s, float x)
{
    return s.scaled(x);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    DoublerObf d;
    outputBuffer[0] = useScaled<DoublerObf>(d, 3.0);
}

// Reached through the imported interface's requirement, so the fallback lowered
// the interface completely.
//CHECK: return {{.*}} * 2.0f;
//CHECK: void computeMain()
