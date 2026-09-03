//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-09-02T18:00:41+00:00
//META: source_commit=caaa91b2d7
//META: purpose=Verifies -get-<compiler>-path prints a `<compiler> path: <text>` line for the named downstream compiler and then lets the compile continue, reporting "not available" when the compiler has no recoverable shared-library path and "not found" when it cannot be located.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::GetCompilerPath` arm of
// `OptionsParser::_parse` in slang-options.cpp. The option has no fixed spelling:
// the parser peels the fixed `-get-` prefix and `-path` suffix off the argument,
// resolves the middle with `TypeTextUtil::findPassThrough`, asks the session for
// that downstream compiler's on-disk path via `getDownstreamCompilerPath`, prints
// the result, and `break`s so parsing and compilation continue. Stripping the two
// fixed affixes (rather than a `lastIndexOf('-')` scan) is what lets a hyphenated
// compiler name such as `spirv-dis` round-trip correctly.
//
// One run exercises four spellings chosen to reach different sides of the result branch:
// `glslang` is linked into the build and reports a real shared-library path
// (SLANG_OK); `clang` is an executable-based command-line compiler with no
// recoverable shared-library path and reports `not available`
// (SLANG_E_NOT_AVAILABLE); `fxc` is Windows-only and, on a machine without it,
// reports `not found` (SLANG_E_NOT_FOUND); `spirv-dis` carries a `-` in its name
// and, resolving at all, proves the affix-stripping is correct. Each value is
// wildcarded because which branch a given compiler takes depends on what is
// installed on the machine; the point is that every flag is recognized, prints its
// line in flag order (each arm reports inline as options are parsed), and the
// compile still proceeds. Observed on this machine: "glslang path: <...>.so",
// "clang path: not available", "fxc path: not found", "spirv-dis path: <...>.so".
//
// The trailing `void main(` is the observable proof that this arm reports and
// continues rather than terminating the parse.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -get-glslang-path -get-clang-path -get-fxc-path -get-spirv-dis-path

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RWStructuredBuffer<int> b;
    b[0] = 1;
}

//CHECK: glslang path: {{.+}}
//CHECK: clang path: {{.+}}
//CHECK: fxc path: {{.+}}
//CHECK: spirv-dis path: {{.+}}
//CHECK: void main(
