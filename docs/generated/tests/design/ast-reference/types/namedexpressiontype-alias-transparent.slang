//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=A NamedExpressionType introduced by `typealias` denotes the aliased type, so a value of the underlying type is accepted wherever the alias is written and vice versa.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a typedef'd name is a type in its own right that still
// denotes its aliased type: a plain `float` is passed to a parameter
// spelled with the alias, and the alias-typed result is consumed as a
// float. CHECK asserts the computed value, which is only reachable if
// the alias resolved to `float` on both sides of the call.

//TEST:INTERPRET(filecheck=CHECK):

typealias Celsius = float;

Celsius warmer(Celsius c) { return c + 5.0f; }

void main()
{
    float raw = 20.0f;
    Celsius c = warmer(raw);
    float back = c;
    // CHECK: c=25
    printf("c=%d\n", int(back));
}
