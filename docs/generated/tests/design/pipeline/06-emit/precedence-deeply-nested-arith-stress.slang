//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#operator-precedence-and-parenthesization
//META: doc_section_digest=71c6e6a3dd9f98cd8871ef646710647c71e8dc0f8d4f04eda428d745f76e66a0
//META: purpose=Stress: a deeply nested arithmetic expression `((a+b)*(c+d)) - (a*b) + c*d` is emitted with the minimum parentheses set — additions stay parenthesised when they sit under a multiplication, but mul-precedence subexpressions are not over-wrapped.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe of the precedence helper: combine the two boundary
// shapes (parens-needed vs parens-omitted) into one expression that
// pressures the helper to decide each sub-expression independently.
// HLSL is the most stable text-emit target for this; the additions
// inside the multiplication keep their parens, while the
// multiplications inside the subtraction/addition drop theirs.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

uniform int a;
uniform int b;
uniform int c;
uniform int d;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = ((a + b) * (c + d)) - (a * b) + c * d;
}

// The emitted expression keeps the `+` parens (required inside `*`)
// and drops the `*` parens (unneeded inside `-`/`+`).
// CHECK: ({{.*}} + {{.*}}) * ({{.*}} + {{.*}}) - {{.*}} * {{.*}} + {{.*}} * {{.*}}
