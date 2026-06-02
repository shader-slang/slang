//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#literals-constant-group
//META: doc_section_digest=d67638b07e9c2bc6f1b10dccded97e8311026aa89ba27291ca4c08770651641c
//META: purpose=A `void`-returning function's terminator is `return_val(void_constant)`, surfacing the unique `void` value the doc lists as `VoidLit`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Literals (`Constant` group) table lists `void_constant`
// (`VoidLit`) as "the unique `void` value". It appears as the
// argument of every `void` function's `return_val` terminator. A
// helper that returns `void` and a `void main` both end with
// `return_val(void_constant)`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

void helper(int x)
{
    buf[1] = x;
    return;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a;
    helper(a + 1);
}

// IR: return_val(void_constant)
// IR-LABEL: func %main
// IR: return_val(void_constant)
