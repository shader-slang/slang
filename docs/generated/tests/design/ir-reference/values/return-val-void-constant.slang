//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#literals-constant-group
//META: doc_section_digest=918df95e795705d19fc754f327c05c827640af724b9f88ca647e64093fd4e807
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
