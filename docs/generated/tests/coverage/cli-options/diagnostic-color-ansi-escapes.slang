//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -diagnostic-color always wraps diagnostic text in ANSI SGR escapes even when the output is not a terminal, while never leaves the plain text form, and an unrecognized value is rejected with E00062.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::DiagnosticColor` arm of
// `OptionsParser::_parse` in slang-options.cpp, which parses `always`/`never`/
// `auto` and records the mode on the option set; `applySettingsToDiagnosticSink`
// in slang-compiler-options.cpp then applies it, resolving `auto` by asking the
// writer whether it is a console.
//
// slang-test captures slangc's output through a pipe, so `auto` — the default —
// resolves to "no color" there. That makes `always` the interesting arm: it is
// the only way to see the coloured rendering under a non-tty, and it is what
// this test pins. The `warning` keyword comes out wrapped in the yellow SGR pair
// `ESC[33m` ... `ESC[0m`, and the whole diagnostic switches to the box-drawing
// frame (`|` becomes `│`). FileCheck patterns cannot spell a literal ESC byte,
// so `{{.}}` stands in for it while the SGR parameter text itself is pinned
// literally.
//
// NEVER pins the plain rendering from the identical source, so the flag is the
// sole cause of the delta, and BADVALUE pins the accepted-value list.

//TEST:SIMPLE(filecheck=ALWAYS):-target hlsl -entry main -stage compute -diagnostic-color always
//TEST:SIMPLE(filecheck=NEVER):-target hlsl -entry main -stage compute -diagnostic-color never
//TEST:SIMPLE(filecheck=BADVALUE):-target hlsl -entry main -stage compute -diagnostic-color bogus

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    int x;
    RWStructuredBuffer<int> b;
    b[0] = x;
}

//ALWAYS: {{.}}[33mwarning{{.}}[0m[E41016]

//NEVER: warning[E41016]: use of uninitialized variable
//NEVER-NOT: [33mwarning

//BADVALUE: error[E00062]: unknown value for option '-diagnostic-color'. Valid values are 'always, never, auto'
