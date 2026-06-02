//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#associated-types
//META: doc_section_digest=8d9c44dd9c721b43aebf4a957962e3afa5853681003d7200add0704f4d3c54f1
//META: purpose=An `associatedtype` in an interface introduces a type requirement; different implementations may satisfy it with different concrete types.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `associatedtype` in an interface introduces a per-implementation type slot.
// IntWrapper satisfies it with `int`; getValue() returns data via ValueType, and 99
// confirms the concrete type resolved correctly at runtime.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasValue
{
    associatedtype ValueType;
    ValueType getValue();
}

struct IntWrapper : IHasValue
{
    typealias ValueType = int;
    int data;
    int getValue() { return data; }
}

void main()
{
    IntWrapper w;
    w.data = 99;
    // CHECK: 99
    printf("%d\n", w.getValue());
}
