//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:11+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-initializer.md#initializer-list-expression
//META: doc_section_digest=68be737ab4b8adb6374f77514e63e2dd70ca264c5b4d8f7ff4cffa68f75e41dc
//META: purpose=A zero-element initializer list {} is a valid initializer list expression (zero or more expressions).
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower boundary of the initializer list element count: zero elements.
// An empty struct initialized with `{}` must compile and produce a usable instance.
// CHECK asserts "ok" is printed, confirming the zero-element initializer list was
// accepted and execution reached past the declaration.

//TEST:INTERPRET(filecheck=CHECK):

struct Empty {};

void main()
{
    Empty e = {};
    // If we reach here without a crash, the zero-element init list was accepted.
    //CHECK: ok
    printf("ok\n");
}
