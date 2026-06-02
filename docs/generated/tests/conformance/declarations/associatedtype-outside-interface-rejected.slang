//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#associated-types
//META: doc_section_digest=8d9c44dd9c721b43aebf4a957962e3afa5853681003d7200add0704f4d3c54f1
//META: purpose=It is an error to declare an `associatedtype` anywhere other than the body of an interface declaration; the compiler rejects it with E30102.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `associatedtype` outside an interface body is a compile error.
// Placing it inside a `struct` must trigger E30102; CHECK-DAG confirms both the
// error severity and the exact diagnostic code.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct Foo
{
    associatedtype Elem;
}

void main() {}

// CHECK-DAG: error
// CHECK-DAG: E30102
