//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -no-codegen still runs the front end and reports its diagnostics but emits no target code, where the same run without the flag emits HLSL.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::NoCodeGen` arm of `OptionsParser::_parse`
// in slang-options.cpp, which is a pure alias — it sets
// `OptionKind::SkipCodeGen` on the linkage rather than a flag of its own.
//
// The alias makes the observable simple: the front end still runs, so the
// uninitialized-variable warning below is still reported, but nothing is emitted
// to the chosen target. DEFAULT pins that the same source does emit HLSL
// (`void main(` plus the `#pragma pack_matrix` prologue), and NOCODEGEN pins the
// warning together with CHECK-NOTs for both of those emission landmarks.
// Everything else about the two runs is identical, so the flag is the sole cause.

//TEST:SIMPLE(filecheck=DEFAULT):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=NOCODEGEN):-target hlsl -entry main -stage compute -no-codegen

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    int x;
    RWStructuredBuffer<int> b;
    b[0] = x;
}

//DEFAULT: warning[E41016]: use of uninitialized variable
//DEFAULT: #pragma pack_matrix
//DEFAULT: void main(

//NOCODEGEN: warning[E41016]: use of uninitialized variable
//NOCODEGEN-NOT: #pragma pack_matrix
//NOCODEGEN-NOT: void main(
