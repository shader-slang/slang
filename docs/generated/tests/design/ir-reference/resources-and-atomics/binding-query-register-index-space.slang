//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#resource-queries-and-modifiers
//META: doc_section_digest=3cfadedc702f1b21642954d604e48414dff40066b7cdd53cde1b9b52264172e7
//META: purpose=Calling the underscore-prefixed `__getRegisterIndex` / `__getRegisterSpace` intrinsics on a sampler surfaces the two `BindingQuery` opcodes `getRegisterIndex(%samp)` and `getRegisterSpace(%samp)` directly in `func %main` at the lowering snapshot.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two `BindingQuery` children of the resource-query table, and
// the accompanying claim that a row whose only surface is an
// underscore-prefixed intrinsic has no wrapper to inline through: the
// opcode is already in `func %main` in the first `-dump-ir` snapshot.
//
// The two results are summed into the buffer store so neither is dead
// before the dump, and the label directive anchoring at `func %main` keeps
// the matches out of the core-module bodies the dump also carries. HLSL is
// the target because
// these queries need a resource whose register/space assignment the
// backend can determine — on CUDA the same shader is rejected with
// `E81111`.

//TEST:SIMPLE(filecheck=IR):-target hlsl -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<uint> outBuf;
SamplerState samp;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = __getRegisterIndex(samp) + __getRegisterSpace(samp);
}

// IR-LABEL: func %main
// IR-DAG: getRegisterIndex(%samp)
// IR-DAG: getRegisterSpace(%samp)
