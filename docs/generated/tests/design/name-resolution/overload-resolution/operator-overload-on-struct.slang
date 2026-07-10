//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#operator-overloading
//META: doc_section_digest=6c67481fccb6a5d96551c93af7509ffafa480d11451126832bec4a0352c4b938
//META: purpose=Operator expressions reach overload resolution through the same OverloadResolveContext machinery as named calls; a user-defined operator+ for a struct fires when both operands are that struct type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "Operator expressions reach overload resolution through the same
// OverloadResolveContext machinery as named calls". The user-defined
// `operator+` for Vec is found by member lookup on the operand types.

//TEST:INTERPRET(filecheck=CHECK):

struct Vec
{
    int x;
    int y;
}

Vec operator+(Vec a, Vec b)
{
    Vec r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}

void main()
{
    Vec a;
    a.x = 1;
    a.y = 2;
    Vec b;
    b.x = 10;
    b.y = 20;
    Vec c = a + b;
    // CHECK: c=11,22
    printf("c=%d,%d\n", c.x, c.y);
}
