//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#resource-queries-and-modifiers
//META: doc_section_digest=3cfadedc702f1b21642954d604e48414dff40066b7cdd53cde1b9b52264172e7
//META: purpose=The three buffer-view resource-query rows whose only surface is an underscore-prefixed core-module intrinsic reach the lowering snapshot with no inlining step in between, so `func %main` itself holds `getEquivalentStructuredBuffer`, `getStructuredBufferPtr`, and `getUntypedBufferPtr`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the claim that these opcodes behave the *opposite* way from the
// `Interlocked*` / `WaveGetActiveMask` wrapper cases: because no ordinary
// Slang function calls the underscore intrinsic on the user's behalf, the
// `__intrinsic_op` sits directly on the declaration the user writes, so the
// opcode is present in the very first `-dump-ir` snapshot rather than
// appearing only once inlining has run.
//
// Each result is folded into the value stored to `outBuf`, so nothing is
// dead-code-eliminated before the dump. The label directive anchoring at
// `func %main` keeps the matches inside the user body — a match in the
// pulled-in core-module bodies would not prove the "no inlining step in
// between" part of the claim. CUDA is the target because
// `__getStructuredBufferPtr` is gated to `cpp_cuda_spirv` and
// `__getByteAddressBufferPtr` to `cpp_cuda`.

//TEST:SIMPLE(filecheck=IR):-target cuda -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<uint> outBuf;
ByteAddressBuffer ba;
RWByteAddressBuffer rwba;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    let structPtr = __getStructuredBufferPtr<uint, DefaultDataLayout>(outBuf);
    let bytePtr = __getByteAddressBufferPtr(ba);
    RWStructuredBuffer<uint> equivalent = __getEquivalentStructuredBuffer<uint>(rwba);

    outBuf[tid.x] = (*bytePtr)[tid.x] + equivalent[tid.x] + (*structPtr)[tid.x];
}

// IR-LABEL: func %main
// IR-DAG: getStructuredBufferPtr(%outBuf)
// IR-DAG: getUntypedBufferPtr(%ba)
// IR-DAG: getEquivalentStructuredBuffer(%rwba)
