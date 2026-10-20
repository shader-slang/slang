//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=b6c75281223e78050e4c294d358e1758fb007b265f058561dae282641f1095c2
//META: purpose=An `internal` decl is visible from any file in the same module; a free function calls another module-local `internal` function successfully.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per-keyword semantics: `internal` is visible inside the declaring
// module. Two free functions in the same module-compilation see
// each other through `getModuleDecl(scope)` equality.

//TEST:INTERPRET(filecheck=CHECK):

internal int helper(int x) { return x + 100; }

void main()
{
    // CHECK: helper=142
    printf("helper=%d\n", helper(42));
}
