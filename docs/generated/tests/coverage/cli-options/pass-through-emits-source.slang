//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -pass-through <compiler> hands the input to the named downstream compiler essentially unmodified, so a source-text target reproduces the original Slang text instead of the normal legalized emit.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::PassThrough` arm of
// `OptionsParser::_parse` in slang-options.cpp, which resolves the operand with
// `TypeTextUtil::findPassThrough` and records the pass-through on the request.
//
// With `-pass-through glslang` Slang stops owning the translation: rather than
// lowering to IR and emitting legalized GLSL it hands the source straight
// through, so what comes out is the file's own text. That is a very legible
// observable — DEFAULT shows the normal emit (a `#version` directive, the
// renamed `b_0`, a `layout(...) buffer` block), while PASSTHROUGH shows the
// untouched Slang declarations, including the `[shader("compute")]` attribute
// and the `RWStructuredBuffer<int> b;` spelling that GLSL has no syntax for.
//
// `glslang` is chosen over `dxc` deliberately: glslang is linked into the build,
// so this run is not tool-gated and validates locally as well as in CI.
//
// Note that pass-through echoes the whole file, comments included, so the
// PASSTHROUGH CHECK-NOT has to name a token that appears only in generated GLSL
// (`layout(local_size_x`) and never in this file's own prose.

//TEST:SIMPLE(filecheck=DEFAULT):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=PASSTHROUGH):-target glsl -entry main -stage compute -pass-through glslang

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    RWStructuredBuffer<int> b;
    b[0] = int(tid.x);
}

//DEFAULT: #version
//DEFAULT: buffer
//DEFAULT-NOT: [shader("compute")]

//PASSTHROUGH-NOT: layout(local_size_x
//PASSTHROUGH: [shader("compute")]
//PASSTHROUGH: RWStructuredBuffer<int> b;
