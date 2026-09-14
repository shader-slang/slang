//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=d6a8e6a85f98f1fe1e7c4215cf8b2b2f8cf50505ce91badfb5ce987b8c7527a4
//META: purpose=`__target_switch` pushes and pops a ScopeDecl around each case group, so a local declared in one case is not visible from any other case of the same statement.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the contrast between the two switch forms. A plain `switch` scopes
// its whole body once (see switch-case-local-visible-in-later-case.slang),
// but `__target_switch` scopes each case group separately. `a` is declared in
// the `hlsl` case and read in the `spirv` case; the per-case ScopeDecl means
// that read cannot resolve, so the checker reports undefined identifier
// (E30015) at the use site rather than accepting it the way the plain form
// does.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int f()
{
    __target_switch
    {
    case hlsl:
        int a = 1;
        return a;
    case spirv:
        return a;
//CHECK:       ^ undefined identifier
//CHECK:       ^ undefined identifier 'a'.
// CHECK: E30015
    default:
        return 0;
    }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = f();
}
