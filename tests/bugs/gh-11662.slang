// Regression test for issue #11662.
//
// A single-module `-O0 -target spirv` compile needs no downstream (slang-glslang /
// SPIRV-Tools) work: there is nothing to optimize, no second module to link, validation is
// off, and no separate debug info is requested. The SPIR-V backend now skips loading the
// `slang-glslang` module in that case and emits the natively generated module directly.
//
// This test guards that the `-O0 -target spirv` code path still produces a complete, valid
// compute module after that gating change. (It cannot exercise the "module physically
// absent" scenario, because the standard test build always bundles slang-glslang; the binary
// SPIR-V output below is disassembled for FileCheck, which does use slang-glslang.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;
    outputBuffer[i] = int(i) * 2 + 1;
}

// A complete compute module is emitted at -O0 without any downstream optimization pass.
// CHECK: OpEntryPoint GLCompute %computeMain
// CHECK: OpExecutionMode %computeMain LocalSize 4 1 1
// CHECK: OpStore
