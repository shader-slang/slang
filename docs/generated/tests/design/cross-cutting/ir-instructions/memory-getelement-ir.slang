//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=4311a2734c94c89f3951c5e93e111cd25e64443cd00a56d067df242df4d2fb08
//META: purpose=Indexed access into an aggregate value lowers to the `getElement` opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `getElement`/`getElementPtr` row. Indexing a fixed
// array *value* by a non-constant `uniform` index lowers to
// `getElement(base, index)` in the IR dump; the runtime index keeps the
// access from being folded to a specific element.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int i;
uniform StructuredBuffer<int> in_buf;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int arr[4] = { in_buf[0], in_buf[1], in_buf[2], in_buf[3] };
    buf[0] = arr[i];
}

// IR-LABEL: func %main
// IR: getElement(%{{.*}}, %{{.*}})
