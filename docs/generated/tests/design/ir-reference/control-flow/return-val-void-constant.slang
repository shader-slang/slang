//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-returns-and-yields
//META: doc_section_digest=7a22c485cd43a8f37c76060d034a3035010f4828abcf759c51b28677698d3453
//META: purpose=A void function's terminator is return_val(void_constant), the unique VoidLit value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A void entry point that simply writes a buffer terminates with
// return_val(void_constant) — the unique void-typed literal serving
// as the synthetic return value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a;
}

// CHECK-LABEL: func %main
// CHECK: return_val(void_constant)
