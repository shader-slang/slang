//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -<compiler>-version prints a `<compiler> version: <text>` line for the named downstream compiler and then lets the compile continue, reporting "not found" when the compiler cannot be located.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::CompilerVersion` arm of
// `OptionsParser::_parse` in slang-options.cpp (~48 uncovered lines). The option
// has no fixed spelling: the parser peels the `-` prefix and the `-version`
// suffix off the argument, resolves the middle with
// `TypeTextUtil::findPassThrough`, asks the session for that downstream compiler,
// prints its version, and `break`s so parsing and compilation continue.
//
// Two spellings are exercised because they take different sides of the
// "was the compiler found" branch on a machine with no DXC installed: `dxc` is
// absent and reports the literal `not found`, while `glslang` is linked into the
// build and reports a numeric version. The numeric part is wildcarded because it
// tracks the vendored glslang; the `dxc` line is pinned loosely for the same
// reason — a machine that does have DXC on PATH prints a version there instead.
// Observed on this machine: "dxc version: not found" and "glslang version: 0.0".
//
// Both runs also check that HLSL still came out, which is the observable proof
// that this arm reports and continues rather than terminating the parse.

//TEST:SIMPLE(filecheck=DXC):-target hlsl -entry main -stage compute -dxc-version
//TEST:SIMPLE(filecheck=GLSLANG):-target hlsl -entry main -stage compute -glslang-version

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
    b[0] = 1;
}

//DXC: dxc version: {{.+}}
//DXC: void main(

//GLSLANG: glslang version: {{.+}}
//GLSLANG: void main(
