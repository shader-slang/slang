//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#rwstructuredbuffergetelementptr
//META: doc_section_digest=07cc97a7459e17e20f0600d7dea0434758c9364177e70232fe8f6681dd755c8f
//META: purpose=Writing `rwbuf[2147483647] = v` (INT_MAX index literal) still surfaces the documented `rwstructuredBufferGetElementPtr` opcode shape; runtime out-of-bounds behavior is undocumented.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: largest positive `int` literal on the lvalue path.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    rwbuf[2147483647] = (int)tid.x;
}

// IR-LABEL: func %main
// IR: rwstructuredBufferGetElementPtr(%rwbuf, 2147483647 : Int)
