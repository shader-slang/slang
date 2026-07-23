// Test that `-Xspirv-opt <flag>` is forwarded to the SPIRV-Tools optimizer and registered on top
// of the `-OX` preset passes, rather than replacing them. See
// https://github.com/shader-slang/slang/issues/12204
//
// `helper` is a separate function so the `-O1` preset's inlining pass is observable: at `-O1` the
// call is inlined away (no OpFunctionCall), at `-O0` it survives. Debug-name instructions (OpName)
// are emitted regardless; the preset keeps them. Passing `-Xspirv-opt --strip-debug` runs
// spirv-opt's strip-debug pass on top of the preset, removing the debug names while the preset's
// inlining still happens -- proving the passthrough argument was applied in addition to the preset.
//
// `-g2` is deliberately NOT used: it would embed this source file (comments included) as an
// OpString, and FileCheck would then match the mnemonics in these comments instead of the real
// instructions.

// `-O1` is explicit because slang-test otherwise injects `-O0`, which would skip the optimizer and
// make the passthrough flag inert (see the O0 case below).
//TEST:SIMPLE(filecheck=BASE): -target spirv-asm -entry computeMain -stage compute -O1
//TEST:SIMPLE(filecheck=STRIP): -target spirv-asm -entry computeMain -stage compute -O1 -Xspirv-opt --strip-debug

// Under explicit `-O0` the optimizer is not run at all, so neither the preset inlining nor the
// passthrough flag applies -- the option is additive to `-OX`, it does not turn the optimizer on.
//TEST:SIMPLE(filecheck=O0): -target spirv-asm -entry computeMain -stage compute -O0 -Xspirv-opt --strip-debug

// At `-O1` the preset keeps the debug names and inlines the call.
// BASE: {{^}}OpName %
// BASE-NOT: {{^}}%{{[0-9]+}} = OpFunctionCall

// With `--strip-debug` on top of `-O1`: the preset still inlines the call, and the extra pass
// removes the debug names. OpEntryPoint appears near the top of every module, so anchoring on it
// first makes the following exclusions scan the entire remainder of the module (the OpName section
// and all function bodies come after it).
// STRIP: {{^}}OpEntryPoint
// STRIP-NOT: {{^}}OpName %
// STRIP-NOT: {{^}}%{{[0-9]+}} = OpFunctionCall

// At `-O0` nothing runs: the call is not inlined and the debug names are not stripped.
// O0: {{^}}OpName %
// O0: {{^}}%{{[0-9]+}} = OpFunctionCall

RWStructuredBuffer<float> outputBuffer;

float helper(uint i)
{
    return float(i) * 2.0f + 1.0f;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = helper(tid.x);
}
