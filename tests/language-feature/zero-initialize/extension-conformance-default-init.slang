// Companion to cross-module-interface-conforming.slang for issue #12917.
//
// An `internal` struct given `IDefaultInitializable` through a `public extension` must not have its
// synthesized `$init()` witness promoted above the (internal) type it constructs; doing so would
// make the witness reference a less visible type (E30604). Zero-initializing the type here must
// still compile, which guards the "direct conformance only" restriction of the #12917 fix.
//
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -zero-initialize -entry computeMain -stage compute

internal struct S
{
    int f;
    __init(int v) { f = v; }
}
public extension S : IDefaultInitializable {}

RWStructuredBuffer<int> outBuf;

// CHECK-NOT: E30604
// CHECK: OpEntryPoint
[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    outBuf[0] = s.f;
}
