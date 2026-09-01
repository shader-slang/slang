//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `__requireCapability(...)` statement lowers into the entry point's capability set (upgrading the profile) without adding an OpCapability to the emitted SPIR-V.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: characterization-unverified=true
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `visitRequireCapabilityStmt`, which turns the statement's capability-name
// tokens into a CapabilitySet and emits a `LateRequireCapability` intrinsic in
// the function body. Two observations prove the statement was lowered rather
// than discarded: the compiler reports that the entry point's profile was
// implicitly upgraded to include the named capability (E41012, naming
// `spvGroupNonUniformArithmetic`), and the module still compiles to SPIR-V.
// Tagged unverified because the inst is consumed for capability bookkeeping
// only -- no `OpCapability GroupNonUniformArithmetic` is emitted for the
// requirement, and no specification statement was found that says whether it
// should be.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    __requireCapability(spvGroupNonUniformArithmetic);
    outputBuffer[0] = 1.0f;
}

// The requirement reached the entry point's capability set.
//CHECK-DAG: warning[E41012]
//CHECK-DAG: spvGroupNonUniformArithmetic
//CHECK-DAG: see call to '__requireCapability'

// ... and the module still emits.
//CHECK-DAG: OpEntryPoint GLCompute
