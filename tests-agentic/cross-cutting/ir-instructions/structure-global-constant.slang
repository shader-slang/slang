//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=3cd430fe84a5b7025d059358578317eab126613a22446f8d9d7f776e96724317
//META: purpose=Boundary probe of the documented module-scope storage opcodes: a `static const` literal declaration lowers to a `globalConstant` IR opcode (sibling of `global_param`).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `globalConstant` alongside `global_var` and
// `global_param` as module-scope storage opcodes. The user-facing
// surface for `globalConstant` is a `static const` at module scope —
// distinct from the `uniform` surface that produces `global_param`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

static const int k = 42;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = k;
}

// IR: %k{{.*}}: Int{{.*}}= globalConstant(42 : Int)
