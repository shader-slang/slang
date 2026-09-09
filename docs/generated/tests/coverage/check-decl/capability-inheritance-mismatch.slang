//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies the two directions of a sub-type/super-type capability mismatch on an inheritance clause: extra capability on the sub-type (E36114) and a missing target atom (E36118).
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: after checking the witness table,
// `SemanticsDeclCapabilityVisitor::visitInheritanceDecl` compares the
// inheritance clause's capability set with the inheriting declaration's. The two
// failure results are reported differently: `AvailableIsNotASuperSetToRequired`
// takes the `UseOfUndeclaredCapabilityOfInheritanceDecl` arm, while
// `RequiredIsMissingAbstractAtoms` reports the sub-type as missing a
// target/stage atom. `Extra` adds a SPIR-V capability its super-type does not
// declare; `Narrow` declares a different target than its super-type. CHECK pins
// both observed messages and the "see declaration of" note each one appends.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

[require(spirv)]
interface IBase
{
        int g();
}

[require(spirv, spvGroupNonUniformBallot)]
struct Extra : IBase
{
        int g() { return 1; }
}

[require(hlsl)]
interface IHlslBase
{
        int h();
}

[require(glsl)]
struct Narrow : IHlslBase
{
        int h() { return 2; }
}

//CHECK: E36114
//CHECK: 'Extra' uses capability 'spvGroupNonUniformBallot' that is incompatable with the supertype
//CHECK: see declaration of 'IBase'
//CHECK: E36118
//CHECK: subtype 'Narrow' must have the same target/stage support as the supertype; 'Narrow' is missing 'hlsl'
//CHECK: see declaration of 'IHlslBase'
