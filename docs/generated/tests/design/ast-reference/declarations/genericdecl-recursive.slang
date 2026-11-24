//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#genericdecl
//META: doc_section_digest=a2a3ea7028c33b102ee0481668bced4a8344b439c1722b28c04420d18b915e69
//META: purpose=Generic declarations compose: a generic struct used as a field of another generic struct specializes when the outer one does.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that specializing an outer generic propagates the type argument to
// a generic field type. Both fields of the outer struct are inner generic
// values; the CHECK asserts a sum computed from both, which requires the inner
// generic to have been specialized at the outer's argument.

//TEST:INTERPRET(filecheck=CHECK):

struct Box<T> { T value; }

struct Pair<T> { Box<T> first; Box<T> second; }

void main()
{
    Pair<int> p;
    p.first.value  = 3;
    p.second.value = 4;
    //CHECK: 7
    printf("%d\n", p.first.value + p.second.value);
}
