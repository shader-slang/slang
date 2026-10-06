//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#genericdecl
//META: doc_section_digest=985b5affb3dd0e12342a333d794d470a82a87f6624d475545de5bdf3de72fbf2
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
