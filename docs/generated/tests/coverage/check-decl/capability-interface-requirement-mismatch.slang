//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies an interface method implementation that declares a capability the requirement does not is rejected with E36110 under -std 2026.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `SemanticsDeclCapabilityVisitor::visitInheritanceDecl` walks
// the witness table and compares each requirement's declared capability set with
// its implementation's. The comparison is skipped entirely for legacy-language
// modules, so this test compiles with `-std 2026`. `S.f` declares an extra
// SPIR-V capability that the requirement `IThing.f` does not, which takes the
// `UseOfUndeclaredCapabilityOfInterfaceRequirement` arm. CHECK pins the observed
// E36110 text and the follow-up note pointing at the requirement declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2026 -target spirv

interface IThing
{
        [require(spirv)]
        int f();
}

struct S : IThing
{
        [require(spirv, spvGroupNonUniformBallot)]
        int f() { return 1; }
}

//CHECK: E36110
//CHECK: 'f' uses capability 'spvGroupNonUniformBallot' that is incompatable with the interface requirement
//CHECK: see declaration of 'f'
