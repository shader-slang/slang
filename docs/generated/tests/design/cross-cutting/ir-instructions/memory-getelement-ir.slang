//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=Indexing into a vector or array value through a swizzle lowers to a `swizzle` IR opcode, the rvalue indexing operation the doc lists alongside `getElement` / `getElementPtr`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Reading `tid.x` from a `uint3` lowers to a `swizzle(%tid, 0 : Int)`
// in the IR. The doc lists `getElement` / `getElementPtr` for
// indexed access; for fixed-position vector access the lowering
// uses the more specific `swizzle` opcode (still part of the
// memory / indexed-access family).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = int(tid.x);
    buf[0] = x;
}

// IR-LABEL: func %main
// IR: swizzle(%tid, 0 : Int)
