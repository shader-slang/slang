//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=eb992431f4d874635d2da973730ce3ff72d018dedbdbec9c0f839f6aea79fa0c
//META: purpose=An array initializer list lowers to a `makeArray` aggregate-constructor opcode with one operand per element.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's statement that an array initializer list such as
// `int a[3] = { 1, 2, 3 }` lowers to `makeArray`. The elements are
// non-constant `uniform` scalars so the dump keeps the aggregate
// constructor with three operands instead of a folded constant.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform int a;
uniform int b;
uniform int c;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int arr[3] = { a, b, c };
    buf[0] = arr[tid.x];
}

// IR-LABEL: func %main
// IR: makeArray(%{{.*}}, %{{.*}}, %{{.*}})
