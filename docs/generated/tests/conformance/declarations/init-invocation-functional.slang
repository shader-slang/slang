//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#initializers
//META: doc_section_digest=42e0c0a3aebf1644284046e5a27a45b490b9bc6e618e09f646a51067c956edd4
//META: purpose=An `__init` declaration defines a way to initialize an instance; it is invoked by calling the enclosing type as if it were a function.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an `__init` is invoked by calling the type as a function, e.g. `MyVector(3.0)`.
// The initializer broadcasts its scalar argument to both fields; CHECK lines assert x and y
// both printed as 3, confirming the initializer ran and set the fields.

//TEST:INTERPRET(filecheck=CHECK):

struct MyVector
{
    float x, y;
    __init(float s) { x = s; y = s; }
}

void main()
{
    MyVector v = MyVector(3.0);
    // CHECK: 3
    // CHECK: 3
    printf("%g\n%g\n", v.x, v.y);
}
