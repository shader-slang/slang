//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=3cd430fe84a5b7025d059358578317eab126613a22446f8d9d7f776e96724317
//META: purpose=A user-defined function lowers to a `func` opcode whose children are basic `block`s, and a call site lowers to `call`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A user-defined `helper` function lowers to a top-level `func`
// instruction whose body is a sequence of `block`s — the doc's
// "Function; children are blocks" claim. A call from `main` lowers
// to the `call` opcode that the values family covers.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int v) { return v + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helper(a);
}

// IR-LABEL: func %helper
// IR: block %{{.*}}
// IR-LABEL: func %main
// IR: call %helper(%{{.*}})
