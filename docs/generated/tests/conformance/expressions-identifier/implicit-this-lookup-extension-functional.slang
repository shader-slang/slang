//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#implicit-lookup
//META: doc_section_digest=81444a04e3158afb0f979b5eaa4e66672a28128589f884b354ac44b03dbff7ad
//META: purpose=Implicit `this` lookup works inside extension methods — bare field names resolve to `this.field`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that implicit `this` lookup works inside an extension method body.
// Bare `width` and `height` in `Box.area()` must resolve to `this.width` and
// `this.height` without explicit qualification. CHECK asserts the result is 24
// (4*6), proving both names resolved through implicit `this` in the extension.
//TEST:INTERPRET(filecheck=CHECK):

struct Box { int width; int height; }

extension Box
{
    int area()
    {
        // 'width' and 'height' are bare identifier expressions resolving via implicit this.
        return width * height;
    }
}

void main()
{
    Box b;
    b.width = 4;
    b.height = 6;
    int a = b.area();
    //CHECK: 24
    printf("%d\n", a);
}
