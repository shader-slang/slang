//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=The doc example `a = b = c + d + e` groups as `a = (b = ((c+d)+e))` — right-assoc `=` with left-assoc `+`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies mixed associativity in one expression: `+` (level 4, left-assoc)
// and `=` (level 14, right-assoc) combine so `a = b = c + d + e` groups as
// `a = (b = ((c+d)+e))`. The arithmetic runs left-to-right first, then both
// assignments propagate right-to-left; CHECK asserts a=6 and b=6.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 0;
    int b = 0;
    int c = 1, d = 2, e = 3;
    a = b = c + d + e;
    // CHECK: mixed-assoc: a=6 b=6
    printf("mixed-assoc: a=%d b=%d\n", a, b);
}
