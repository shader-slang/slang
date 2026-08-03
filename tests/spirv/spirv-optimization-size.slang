// Test the `-Os` optimization level, which asks the SPIR-V optimizer for the smallest output
// rather than the fastest, by running the same passes as `spirv-opt -Os`. See
// https://github.com/shader-slang/slang/issues/12331
//
// `-Os` is not a further point on the `-O0`..`-O3` speed scale, it selects a different
// optimization goal, so it gets its own arm in `glslang_optimizeSPIRV` (slang-glslang.cpp)
// rather than being folded into one of the existing levels. Two things need testing here: that
// the new spellings reach that arm at all, and that the arm registers a real preset instead of
// falling through to the "no preset" or default behavior.
//
// This shader gives both an inter-procedural and an intra-procedural handle on that. `helper` is
// a separate function, so the preset's inlining and dead-function elimination are observable as
// the disappearance of `OpFunctionCall` and of `helper`'s own `OpFunction`. The `if`/`else` is a
// separate handle: the size preset's branch-elimination and CFG-cleanup passes collapse it, so
// the `OpBranchConditional` disappears and only the `OpSelect` that Slang already emitted for the
// value remains.
//
// `-g2` is deliberately NOT used: it would embed this source file (comments included) as an
// OpString, and FileCheck would then match the mnemonics in these comments instead of the real
// instructions.

// The short and long spellings of the level must both select it. `-Osize` is checked separately
// rather than assumed, because the spellings come from a comma-separated name list
// (`s_optimizationLevels` in slang-type-text-util.cpp) where only the first entry is canonical.
//TEST:SIMPLE(filecheck=SIZE): -target spirv-asm -entry computeMain -stage compute -Os
//TEST:SIMPLE(filecheck=SIZE): -target spirv-asm -entry computeMain -stage compute -Osize

// `-O0` and `-O1` are the contrast cases that make the SIZE checks meaningful. `-O0` registers no
// preset at all, and `-O1`'s preset does inline but does not include the branch-elimination and
// CFG-cleanup passes that the size preset runs -- so at both of these the `OpBranchConditional`
// survives. If `-Os` ever regressed into either of those arms, the SIZE checks below would fail.
//TEST:SIMPLE(filecheck=O0): -target spirv-asm -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=O1): -target spirv-asm -entry computeMain -stage compute -O1

// The level also has to survive being written back out as a command line. `CompilerOptionSet::
// writeCommandLineArgs` records the options into the SPIR-V debug information, and it used to
// write the level as its raw integer -- which for this level would produce `-O4`, a spelling the
// `-O` parser rejects. This case is the only place that serialization is observable from a test.
//
// This is the one case that uses `-g2`, which embeds this source file (comments included) as an
// OpString. That is safe here because the check below anchors on the OpString that *starts* with
// the recorded command line, which no embedded source line can do.
//TEST:SIMPLE(filecheck=CMDLINE): -target spirv-asm -entry computeMain -stage compute -Os -g2

// A level spelling that does not exist is rejected, and the error lists the valid spellings --
// including the new `s` and `size`. This is what proves the name table drives the parser, rather
// than `-Os` being special-cased somewhere in the `-O` parsing.
//TEST:SIMPLE(filecheck=BADLEVEL): -target spirv-asm -entry computeMain -stage compute -Oz

RWStructuredBuffer<float> outputBuffer;

float helper(float x)
{
    return x * 2.0f;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float v;
    if (tid.x > 4)
        v = 1.0f;
    else
        v = 2.0f;

    outputBuffer[tid.x] = helper(v);
}

// Under `-Os` the preset inlines `helper` and eliminates it as dead (so neither the call nor the
// function definition remains), and collapses the conditional (so no `OpBranchConditional`).
// Anchoring on `OpEntryPoint` first makes the exclusions scan the rest of the module: the SPIR-V
// logical layout guarantees `OpEntryPoint` precedes the debug, type and function-body sections,
// so this is a layout guarantee rather than incidental emitter ordering.
// SIZE: {{^}}OpEntryPoint
// SIZE-NOT: {{^}}%helper = OpFunction
// SIZE-NOT: OpFunctionCall
// SIZE-NOT: {{^}}{{.*}}OpBranchConditional

// At `-O0` no preset runs: the branch survives and so does the call. The checks follow the order
// the instructions appear in the function body (the conditional comes before the call that
// consumes its result), because FileCheck matches positive directives in sequence.
// O0: {{^}}OpEntryPoint
// O0: OpBranchConditional
// O0: OpFunctionCall

// At `-O1` the preset inlines the call, but its pass list stops short of the branch elimination
// and CFG cleanup the size preset runs, so the conditional branch is still there. This is the
// check that distinguishes `-Os` from the default preset.
// O1: {{^}}OpEntryPoint
// O1-NOT: OpFunctionCall
// O1: OpBranchConditional

// The recorded command line spells the level `-Os`, not `-O4`. The `-I` argument in between
// carries an absolute path, so it is skipped with a wildcard.
// CMDLINE: OpString "-target spirv{{.*}} -Os

// An unknown `-O` value is a hard error listing every valid spelling, so a regression that
// dropped `s`/`size` from the table would change this line.
// BADLEVEL: error{{.*}}unknown value for option '-O'
// BADLEVEL-SAME: 3, maximal, s, size
