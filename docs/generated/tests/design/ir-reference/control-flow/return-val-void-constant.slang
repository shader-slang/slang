//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-returns-and-yields
//META: doc_section_digest=f192f41be797c74b1114b7ff57ec1c86180c872b87ad52a64a206e75eec3e27e
//META: purpose=A void function's terminator is return_val(void_constant), the unique VoidLit value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A void entry point that simply writes a buffer terminates with
// return_val(void_constant) — the unique void-typed literal serving
// as the synthetic return value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a;
}

// CHECK-LABEL: func %main
// CHECK: return_val(void_constant)
