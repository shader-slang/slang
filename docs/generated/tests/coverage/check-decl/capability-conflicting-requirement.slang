//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a function that calls one hlsl-only and one glsl-only callee is rejected with E36100 naming the conflicting callee and the caller's already-inferred set.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `_propagateRequirement` joins each referenced declaration's
// capability set into the enclosing declaration's inferred set. When that join
// turns a previously valid set invalid, and the reference came from a
// declaration, it reports `ConflictingCapabilityDueToUseOfDecl` against the
// second reference rather than the first. `mixed` below calls an `hlsl`-only
// function and then a `glsl`-only one, so the diagnostic lands on the `onlyGlsl`
// call. CHECK matches the code and the distinguishing head of the message,
// leaving the enumerated stage list unmatched since it is incidental here.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

[require(hlsl)]
void onlyHlsl()
{
}

[require(glsl)]
void onlyGlsl()
{
}

void mixed()
{
        onlyHlsl();
        onlyGlsl();
//CHECK:^^^^^^^^ E36100
//CHECK:^^^^^^^^ 'onlyGlsl' requires capability
}
