//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#val-nodebase
//META: doc_section_digest=17754b4ad55bb3bbeb3b934dd617e4a3120a865a281a81848bf3fc5ab03948d7
//META: purpose=A use of a name must resolve to a declaration before a DeclRefBase can be formed; when no reachable declaration matches, the checker reports an undefined identifier.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative side of decl-ref formation: an identifier with
// no matching declaration in any reachable scope cannot become a
// reference, and the checker says so. CHECK pins the E30015 code and
// the caret span covering the whole unresolved name.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int v = doesNotExistAnywhere();
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ undefined identifier 'doesNotExistAnywhere'.
// CHECK: E30015
}
