//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#source-location-serialization
//META: doc_section_digest=96fd282fcc078777e024afffe532377747fb1c684cee2518cb1623dada245df2
//META: purpose=Whether and how a #line directive appears in emitted text is the compile option -line-directive-mode, which takes none, source-map, default, standard or glsl; any other value is rejected.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says the spelling of emitted #line directives is a compile
// option, not a property of the serialized locations, and lists the five
// accepted values. STANDARD and DEFAULT assert the path-bearing spelling,
// GLSL the numeric file id, NONE the absence, and BAD asserts a sixth
// value is rejected with the option diagnostic (exit code 1 for a bad
// command-line option) that lists the valid values.

//TEST:SIMPLE(filecheck=STANDARD):-target hlsl -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=STANDARD):-target hlsl -entry main -stage compute -line-directive-mode default
//TEST:SIMPLE(filecheck=NONE):-target hlsl -entry main -stage compute -line-directive-mode source-map
//TEST:SIMPLE(filecheck=STANDARD):-target glsl -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=STANDARD):-target metal -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=STANDARD):-target wgsl -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=BAD, exit-code=1):-target hlsl -entry main -stage compute -line-directive-mode bogus

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 7;
}

// STANDARD: #line {{[0-9]+}} "{{.*}}line-directive-mode-accepted-values.slang"

// SPIRV: OpEntryPoint GLCompute

// NONE-NOT: #line
// NONE: void main(

// BAD: error[E00062]: unknown value for option '-line-directive-mode'. Valid values are 'none, source-map, default, standard, glsl'
