//TEST:COMPILE: tests/spirv/debug-declare-module-lib.slang -o tests/spirv/debug-declare-module-lib.slang-module -g2
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -g0
//TEST:SIMPLE(filecheck=STRIP): -target spirv-asm -entry computeMain -stage compute -g0 -dump-ir-after stripDebugInfo

// Regression test for https://github.com/shader-slang/slang/issues/13841
// A module compiled at -g2 and linked at -g0 has its debug information stripped, and with it the
// `DebugVarStorage` decorations of its storage, which would otherwise name the removed debug
// variables. No debug declaration reaches the output.

import debug_declare_module_lib;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float v = outputBuffer[0];
    stepped(v, int(tid.x));
    outputBuffer[0] = v;
}

// CHECK: OpEntryPoint
// CHECK-NOT: DebugDeclare
// CHECK-NOT: _dbgvar_

// STRIP: ### AFTER stripDebugInfo:
// STRIP-NOT: DebugVarStorage
