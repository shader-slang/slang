//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies calling a function that requires an undeclared SPIR-V capability from a function that only declares `spirv` is rejected with E36104 plus provenance notes.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: when the atoms missing from a caller's declared capability
// set are not stage/target atoms, `diagnoseUndeclaredCapability` takes the
// `UseOfUndeclaredCapability` arm and names the missing atom directly. CHECK
// pins the observed E36104 text (including the atom name
// `spvGroupNonUniformBallot`) and the two provenance notes that
// `diagnoseCapabilityProvenance` walks back to the callee that requires it. The
// check is exhaustive, so it also pins that no note is emitted for `main`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

[require(spvGroupNonUniformBallot)]
void needsBallot()
{
}

[require(spirv)]
void spirvCaller()
{
        needsBallot();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
        spirvCaller();
}

//CHECK: E36104
//CHECK: 'spirvCaller' uses undeclared capability 'spvGroupNonUniformBallot'
//CHECK: see using of 'needsBallot'
//CHECK: see definition of 'needsBallot'
