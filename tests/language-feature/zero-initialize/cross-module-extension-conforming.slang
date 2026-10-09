// Regression test for https://github.com/shader-slang/slang/issues/12917.
//
// Companion to cross-module-interface-conforming.slang. Here `Concrete` (with an explicit value
// ctor, so no member-wise default ctor is synthesized) receives `IDefaultInitializable` through a
// `public extension` in its defining module — the reporter's documented workaround — rather than
// as a direct conformance. Default-constructing the imported `Concrete` field cross-module under
// `-zero-initialize` must still compile cleanly (no spurious `E30600`).
//
// This guards the extension shape against a #12917-style regression; it does NOT assert that the
// extension's witness resolves across the boundary. It compiles clean for a different reason than
// the direct case: the extension's `$init()` witness is attached to the `ExtensionDecl`, so
// `_getDefaultCtor` (which scans only the struct's direct members) never finds it, and default
// construction falls through to a plain `DefaultConstructExpr` that performs no source-level
// visibility check.
//
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -zero-initialize -entry computeMain -stage compute

import "cross-module-extension-conforming-mod";

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
