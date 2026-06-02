//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#subscripts
//META: doc_section_digest=96fe50283b71fe44fe9a09f0fc0cff5190f23935a1a109bbcdaf1cb870ea319d
//META: purpose=A `__subscript` declaration with a `get` accessor enables `[]` syntax; reading via `[]` invokes the getter.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `__subscript` with a `get` accessor enables `[]` read syntax.
// `v[0]` and `v[1]` call the getter which branches on the index; CHECK lines assert
// 5 and 9, confirming both accesses invoked the getter with the correct index.

//TEST:INTERPRET(filecheck=CHECK):

struct MyVec
{
    float x, y;
    __subscript(int index) -> float
    {
        get { return index == 0 ? x : y; }
    }
}

void main()
{
    MyVec v;
    v.x = 5.0;
    v.y = 9.0;
    // CHECK: 5
    // CHECK: 9
    printf("%g\n%g\n", v[0], v[1]);
}
