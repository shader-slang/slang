//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=b3984feebdaa7987fca66b0f53fdce865b89b787721b062d24304277154e8746
//META: purpose=Boundary probe of the `block` family: an entry function with an empty body still produces a basic `block` ending in `return_val(void_constant)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc describes `block` as a basic block whose first N children
// are `param`s. The minimal `block` is one with no instructions
// before its terminator — an empty entry-point body. The terminator
// is still required and visible in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
}

// IR-LABEL: func %main
// IR: return_val(void_constant)
