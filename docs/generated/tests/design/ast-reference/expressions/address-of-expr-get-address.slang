//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies that `__getAddress(e)` yields a pointer to the operand's storage, so writes through the pointer are visible in the original variable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The address-of expression has its own keyword spelling rather than sharing
// the unary `&` operator. A struct local's address is taken, a field is
// written through the resulting pointer, and the original local is read back;
// the CHECK asserts the mutated value, which only appears if the pointer
// aliases the same storage.

//TEST:INTERPRET(filecheck=CHECK):

struct S { int v; }

void main()
{
    S s = { 7 };
    S* p = __getAddress(s);
    (*p).v = 11;
    // CHECK: v=11
    printf("v=%d\n", s.v);
}
