//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=53cc9594629df0d6fdaba40b6892c4ce92119b3c38da01e7da3565a3cdc7634f
//META: purpose=A catch handler is parsed inside a fresh ScopeDecl, so the error variable it binds is not visible after the do/catch statement.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the boundary of the scope the parser pushes for a catch handler.
// `catch(err: MyError)` binds `err` inside that scope and the handler body
// reads `err.code`, which must check cleanly; the following statement at the
// enclosing function scope reads the same name and must not resolve. The
// annotated E30015 on `err` after the statement is what pins the boundary.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct MyError
{
    int code = 0;
};

int mayFail(int v) throws MyError
{
    if (v < 0)
        throw MyError(7);
    return v;
}

void main()
{
    int r = 0;
    do
    {
        r = try mayFail(1);
    }
    catch (err: MyError)
    {
        r = err.code;
    }
    int q = err.code;
//CHECK:    ^^^ undefined identifier
//CHECK:    ^^^ undefined identifier 'err'.
// CHECK: E30015
    printf("%d\n", r + q);
}
