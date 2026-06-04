//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-1 postfix `[]` binds more tightly than level-2 prefix `!`/`-`: `-arr[1]` groups as `-(arr[1])`, not `(-arr)[1]`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The precedence table shows postfix ops (including `[]`) at level 1,
// prefix/unary at level 2. Level 1 is lower numeric = higher precedence,
// so `[]` subscript binds before prefix `-`.
// `-arr[1]` = `-(arr[1])` = -(element at index 1), not `(-arr)[1]`.

//TEST:INTERPRET(filecheck=CHECK):

int neg_subscript(int a, int b)
{
    int arr[2];
    arr[0] = a;
    arr[1] = b;
    return -arr[1];
}

void main()
{
    // arr[1]=7, so -arr[1] = -(7) = -7
    int result = neg_subscript(3, 7);
    // CHECK: postfix-over-prefix: -7
    printf("postfix-over-prefix: %d\n", result);
}
