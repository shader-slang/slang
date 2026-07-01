//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-6 relational `<` binds more tightly than level-7 `==`: `1 < 2 == 3 < 4` groups as `(1<2) == (3<4)` = `true == true` = true.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The precedence table says `<`/`<=`/`>`/`>=` are level 6, `==`/`!=` are level 7.
// Level 6 is lower numeric = higher precedence, so relational binds before equality.
// `1 < 2 == 3 < 4` groups as `(1<2) == (3<4)` = `true == true` = true.
// Wrong grouping `1 < (2==3) < 4` would be a type error or `1 < false < 4`.

//TEST:INTERPRET(filecheck=CHECK):

int relational_eq(int a, int b, int c, int d)
{
    return (a < b == c < d) ? 1 : 0;
}

void main()
{
    int r1 = relational_eq(1, 2, 3, 4); // (1<2)==(3<4) = true==true = 1
    int r2 = relational_eq(2, 1, 4, 3); // (2<1)==(4<3) = false==false = 1
    // CHECK-DAG: relational-over-equality r1=1
    // CHECK-DAG: relational-over-equality r2=1
    printf("relational-over-equality r1=%d\n", r1);
    printf("relational-over-equality r2=%d\n", r2);
}
