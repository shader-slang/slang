//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T20:50:17+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#nonuniformresourceindex
//META: doc_section_digest=da7804ffd61b86af2c970fd765eb12daa92bd5d94d9373a5ac2b08752465a66f
//META: purpose=The deprecated `NonUniformResourceIndex<T>` overload for a non-arithmetic type warns (`E31200`) and creates no `nonUniformResourceIndex` opcode; it returns its argument unchanged.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A bool is not a __BuiltinArithmeticType, so overload resolution selects the
// deprecated early-inlined overload and the lowering snapshot of main has
// no nonUniformResourceIndex instruction.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool b = NonUniformResourceIndex(tid.x == 0);
    outBuf[0] = b ? 1u : 2u;
}

// IR: E31200
// IR-LABEL: func %main
// IR-NOT: nonUniformResourceIndex
// IR: return_val
