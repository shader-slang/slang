//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=68a2d43cee4f2785028c9292caeaac660921efdc95b8ec2b5eface8a8b89a0dc
//META: purpose=Member lookup that exhausts the inheritance walk without finding a candidate produces the 'member not found' diagnostic (code 30027), naming both the missing identifier and the host type.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Member lookup walks the facets of the type's inheritance chain.
// When none of them produces a candidate, the checker emits
// `30027` "member not found" with the missing name and host type
// interpolated.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct A
{
    int x;
}

void main()
{
    A a;
    int n = a.missing;
//CHECK:      ^^^^^^^ member not found
//CHECK:      ^^^^^^^ 'missing' is not a member of 'A'.
// CHECK: E30027
}
