//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#subscripts
//META: doc_section_digest=96fe50283b71fe44fe9a09f0fc0cff5190f23935a1a109bbcdaf1cb870ea319d
//META: purpose=The result type clause of a `__subscript` declaration cannot be elided; a `__subscript` without `->` is a compile error (E30901).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "The result type clause of a subscript declaration cannot be elided."
// Without a `->` return type, slangc emits E30901.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct Bad
{
    __subscript(int i)
    {
        get { return 0; }
    }
}

void main() {}

// CHECK-DAG: error
// CHECK-DAG: E30901
