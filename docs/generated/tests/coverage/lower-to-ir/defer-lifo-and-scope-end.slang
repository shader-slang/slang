//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:43:23Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=Verifies multiple `defer` blocks in one scope run in LIFO order at scope end.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the observed lowering of `visitDeferStmt`: each `defer` registers its
// block against the enclosing scope-end (`context->scopeEndBlock`), and the
// blocks fire when the scope is left, in reverse order of declaration. The
// body prints "body" first, then on scope exit "second" then "first" (LIFO),
// then "end". Runs under slangi (control flow here avoids the known VM
// short-circuit bug; no `&&`/`||`/`?:`).
//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    {
        defer { printf("first "); }
        defer { printf("second "); }
        printf("body ");
    }
    printf("end\n");
    //CHECK: body second first end
}
