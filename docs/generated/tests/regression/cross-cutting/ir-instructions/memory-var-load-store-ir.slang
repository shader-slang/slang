//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=A local struct variable lowers to `var` and its field reads/writes lower to `store` / `load` / `get_field_addr` IR opcodes.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A locally declared `Foo` is allocated via `var` (the doc's
// "Local variable allocation; result is Ptr<T>" claim). The
// subsequent field assignment lowers to `get_field_addr` plus a
// `store`, and the read-back to `load` plus `get_field_addr`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Foo { int x; int y; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Foo f;
    f.x = a;
    f.y = a + 1;
    buf[0] = f.x + f.y;
}

// IR-LABEL: func %main
// IR: var
// IR: get_field_addr(%{{.*}}, %{{.*}})
// IR: store(%{{.*}}, %{{.*}})
// IR: load(%{{.*}})
