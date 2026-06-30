// This test verifies that compilation still succeeds in Slang 2026 mode,
// even if an imported module uses default initialization.
//
// This is a cross-module addition to tests/bugs/9191-interface-initialization.slang.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -std 2026

import "cross-module-interface-var-default-initialization-helper.slang";


[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Note: The related warning/error about interface-typed field initialization
    // belongs to the defining module
    StructWithIfaceField ret = { };
}

// Check that things compile, even if the imported module uses
// default-initialized interface variable, which is illegal in Slang 2026.
//
//CHECK: ; Function computeMain
