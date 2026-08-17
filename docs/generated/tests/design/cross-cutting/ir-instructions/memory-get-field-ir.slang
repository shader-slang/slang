//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=88ac9c5bf3929b2eb7c6c8eba693545ca9ae827a013044241007c9ad7639db78
//META: purpose=An rvalue read of a struct field lowers to the `get_field` opcode (`FieldExtract`).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `get_field`/`FieldExtract` row: reading a field of
// a struct *value* (not through a pointer) lowers to `get_field(base,
// key)`. The struct arrives by value from a `StructuredBuffer` load so
// the field read is the rvalue `get_field` form rather than the
// `get_field_addr` lvalue form.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Foo { int x; int y; }

uniform StructuredBuffer<Foo> in_buf;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Foo f = in_buf[0];
    buf[0] = f.x;
}

// IR-LABEL: func %main
// IR: get_field(%{{.*}}, %{{.*}})
