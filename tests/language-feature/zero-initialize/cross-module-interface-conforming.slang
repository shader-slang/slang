// Regression test for https://github.com/shader-slang/slang/issues/12917
//
// Under `-zero-initialize`, materializing this module's zero-initializer default-constructs the
// `Concrete` field imported from another module, which resolves the synthesized
// `IDefaultInitializable.$init()` witness on the `public` `Concrete` across the module boundary.
// That witness must be visible from here, so this compiles cleanly (no spurious `E30600`).
//
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -zero-initialize -entry computeMain -stage compute

import "cross-module-interface-conforming-mod";

// CHECK-NOT: E30600
// CHECK: OpEntryPoint

public struct Container
{
    public Concrete field;
}

RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    Container c;
    outBuf[0] = c.field.get();
}
