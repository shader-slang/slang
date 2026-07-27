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

// These two cases use `-O1` to exercise the preset: BASE shows the preset alone (inlines the call,
// keeps debug names) and STRIP shows the passthrough `--strip-debug` running on top of that preset
// (still inlined, names now removed). `-O1` is explicit because slang-test otherwise injects `-O0`,
// which has no preset -- the `-O0` case below covers passthrough-without-a-preset separately.
//TEST:SIMPLE(filecheck=BASE): -target spirv-asm -entry computeMain -stage compute -O1
//TEST:SIMPLE(filecheck=STRIP): -target spirv-asm -entry computeMain -stage compute -O1 -Xspirv-opt --strip-debug

// `-Xspirv-opt` runs the selected passes even at `-O0`: `--strip-debug` removes the debug names.
// `-O0` has no preset, so the passthrough passes are the only ones that run -- the call is NOT
// inlined (OpFunctionCall survives), which distinguishes this from the `-O1` preset behavior.
//TEST:SIMPLE(filecheck=O0): -target spirv-asm -entry computeMain -stage compute -O0 -Xspirv-opt --strip-debug

// An invalid pass flag is rejected by SPIRV-Tools rather than silently ignored: the optimize step
// fails and its diagnostic is surfaced (the compiler does not run a partially-registered pipeline).
//TEST:SIMPLE(filecheck=INVALID): -target spirv-asm -entry computeMain -stage compute -O1 -Xspirv-opt --this-pass-does-not-exist

// Every `-Xspirv-opt` occurrence accumulates: the forwarding collects a list, not just the first
// flag. A valid first flag plus an invalid second one must still fail on the second -- proving the
// whole list is forwarded (a first-only or mis-counted regression would ignore the second and
// succeed).
//TEST:SIMPLE(filecheck=MULTIFLAG): -target spirv-asm -entry computeMain -stage compute -O1 -Xspirv-opt --strip-debug -Xspirv-opt --this-pass-does-not-exist

// Only `-Xspirv-opt` args are forwarded to the optimizer; `-Xglslang` args target a different
// downstream tool and must not affect a `-target spirv` compile. `-Xglslang --strip-debug` must
// therefore be a no-op here -- the debug names survive and the compile succeeds, rather than the
// flag being misrouted into the optimizer (which would strip the names or fail as an unknown flag).
// (This exercises the direct-SPIR-V arg routing; the `SLANG_PASS_THROUGH_SPIRV_OPT` instance gate
// itself only matters on the `-emit-spirv-via-glsl` path, which is out of scope -- see the docs.)
//TEST:SIMPLE(filecheck=XGLSLANG): -target spirv-asm -entry computeMain -stage compute -O1 -Xglslang --strip-debug

// At `-O1` the preset keeps the debug names and inlines the call.
// BASE: {{^}}OpName %
// BASE-NOT: {{^}}%{{[0-9]+}} = OpFunctionCall

// With `--strip-debug` on top of `-O1`: the preset still inlines the call, and the extra pass
// removes the debug names. The SPIR-V logical layout guarantees OpEntryPoint precedes the debug
// (OpName) and function-body sections, so anchoring on it first makes the following exclusions scan
// the entire remainder of the module -- this is a layout guarantee, not incidental emitter order.
// STRIP: {{^}}OpEntryPoint
// STRIP-NOT: {{^}}OpName %
// STRIP-NOT: {{^}}%{{[0-9]+}} = OpFunctionCall

// At `-O0` the passthrough `--strip-debug` still runs (debug names gone), but there is no preset,
// so the call is not inlined (OpFunctionCall survives). In SPIR-V module layout OpEntryPoint comes
// first, the debug-name (OpName) section next, then the function bodies -- so anchoring on
// OpEntryPoint and asserting no OpName appears before the surviving OpFunctionCall covers the
// stripped debug section and the un-inlined call in one ordered scan.
// O0: {{^}}OpEntryPoint
// O0-NOT: {{^}}OpName %
// O0: {{^}}%{{[0-9]+}} = OpFunctionCall

// The invalid flag is reported by spirv-opt and the compile fails: assert both the failing result
// code (a successful compile would print `result code = 0`) and the surfaced diagnostic, so the
// test fails if the error is emitted but the compile wrongly reports success. `result code = -1`
// and the `Unknown flag` wording are external contracts (slang-test's harness and SPIRV-Tools'
// message); if either drifts, this is wording drift, not a Slang regression -- the `result code =
// -1` and `{{[Ee]rror}}` lines together guard the intent (an invalid flag surfaces an error and
// fails the compile).
// INVALID: result code = -1
// INVALID: {{[Ee]rror}}
// INVALID: Unknown flag

// The second `-Xspirv-opt` flag is the invalid one, so the failure names it -- proving the second
// list element was forwarded, not dropped.
// MULTIFLAG: result code = -1
// MULTIFLAG: Unknown flag {{.*}}this-pass-does-not-exist

// `-Xglslang --strip-debug` is not routed to the optimizer, so the debug names survive and the
// compile succeeds.
// XGLSLANG: result code = 0
// XGLSLANG: {{^}}OpName %

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
