//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#partiallyappliedgenericexpr
//META: doc_section_digest=41b116baf66ac874e944b3604437b71191e5e88d4ceada02bb592fddeca6502a
//META: purpose=A partially applied generic preserves the explicitly supplied argument prefix in order: `triple<int, float>` fixes T and U and the third parameter V is inferred from the call site.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that when a generic is applied to fewer arguments than it
// declares, the explicit arguments bind the leading parameters in order
// and only the remainder is inferred. `triple<int, float>` must pin
// T=int and U=float (so the 2.5f argument stays a float, truncating to
// 2 only at the printf cast), while V=uint comes from the third
// argument. CHECK asserts all three field values.

//TEST:INTERPRET(filecheck=CHECK):

struct Triple<T, U, V>
{
    T a;
    U b;
    V c;
}

Triple<T, U, V> triple<T, U, V>(T a, U b, V c)
{
    Triple<T, U, V> t;
    t.a = a;
    t.b = b;
    t.c = c;
    return t;
}

void main()
{
    // T and U are the explicit prefix; V is inferred from 9u.
    Triple<int, float, uint> t = triple<int, float>(3, 2.5f, 9u);
    // CHECK: a=3
    printf("a=%d\n", t.a);
    // CHECK: b=2
    printf("b=%d\n", int(t.b));
    // CHECK: c=9
    printf("c=%u\n", t.c);
}
