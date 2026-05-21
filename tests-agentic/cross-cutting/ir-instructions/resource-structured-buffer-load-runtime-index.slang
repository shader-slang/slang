//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=fcbecb6413e5961564d51f36c89d78b13abfa2520a8c4c8b2974af9432eaecdb
//META: purpose=Boundary probe of `structuredBufferLoad` with a runtime (non-literal) index: the index operand survives in the IR as a use of the `uniform`-fed value rather than being folded.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `structuredBufferLoad` opcode takes a variadic index. The
// literal-index form (covered by the bootstrap) folds to a constant;
// a `uniform`-fed runtime index instead appears as the SSA value of
// the loaded parameter, pinning the documented `(base, index)`
// operand shape with a non-constant index.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int idx;
uniform StructuredBuffer<int> in_buf;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = in_buf[idx];
}

// IR-LABEL: func %main
// IR: structuredBufferLoad(%inx5Fbuf, %idx)
