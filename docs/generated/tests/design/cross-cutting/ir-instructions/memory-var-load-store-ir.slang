//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=4311a2734c94c89f3951c5e93e111cd25e64443cd00a56d067df242df4d2fb08
//META: purpose=A local struct variable lowers to `var` and its field reads/writes lower to `store` / `load` / `get_field_addr` IR opcodes.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's memory-instruction rows in one shot. A locally
// declared struct `Foo` is allocated via `var` ("Local variable
// allocation; result is Ptr<T>"); a field assignment lowers to
// `get_field_addr` plus `store`; and the read-back uses `load`.

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
