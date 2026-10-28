//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -g2 -emit-spirv-directly
//TEST:SIMPLE(filecheck=CU):-target spirv -entry main -stage compute -g2 -emit-spirv-directly
//TEST:SIMPLE(filecheck=SRC):-target spirv -entry main -stage compute -g2 -emit-spirv-directly -preserve-embedded-source

// Regression test for shader-slang/slang#13982. The `main` entry point uses nothing from the first
// imported module, and uses the second only for a constant whose value is folded into the use
// site. Neither module contributes a function, type, or variable with debug info to `main`, so the
// SPIR-V for `main` must hold the debug records of this file only: one DebugSource and one
// DebugCompilationUnit, with neither module's source text embedded, so that the output does not
// depend on imports that change nothing. The second entry point, which is not compiled here, calls
// into the first module and into a header that module `#include`s, so this file's own IR also
// refers to both of those files.
//
// The CHECK and CU runs see the output with the embedded source text stripped, which slang-test
// does by default. The SRC run keeps that text, and splits each marker pattern with a
// one-character regex group so that it does not match the text of this file.

import debug_source_unused_import_module;
import debug_source_unused_import_const_module;

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) + constModuleMarker13982;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void otherMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(unusedModuleMarker13982(float(tid.x)) + unusedHeaderMarker13982(1.0));
}

// All OpString instructions precede the first DebugSource in the SPIR-V module layout.
// CHECK: [[FILE:%[0-9]+]] = OpString "{{.*}}debug-source-unused-import.slang"
// CHECK-NOT: OpExtInst %void %{{[0-9]+}} DebugSource
// CHECK: OpExtInst %void %{{[0-9]+}} DebugSource [[FILE]]
// CHECK-NOT: OpExtInst %void %{{[0-9]+}} DebugSource

// CU-COUNT-1: OpExtInst %void %{{[0-9]+}} DebugCompilationUnit
// CU-NOT: OpExtInst %void %{{[0-9]+}} DebugCompilationUnit

// SRC-NOT: unusedModule{{M}}arker13982(float x)
// SRC-NOT: unusedHeader{{M}}arker13982(float x)
// SRC-NOT: public static const int constModule{{M}}arker13982
