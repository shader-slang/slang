//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#operator-overloading
//META: doc_section_digest=017e2a9ca7d5b435c5665a1115eba51fbc2b30352078a8ab4c0e65601bfa2273
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
