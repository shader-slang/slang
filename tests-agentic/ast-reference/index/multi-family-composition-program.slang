//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:41Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/ast-reference/index.md#family-taxonomy
//META: doc_section_digest=ee941d37430adef45b384cb8db792e24ac11919541c2f1279e411361abc290a4
//META: purpose=A working program composes concrete leaves from every AST root family the index taxonomy names (Decl, Expr, Stmt, Type, Modifier, Val), demonstrating that the family partition spans the surface language.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The #family-taxonomy mermaid diagram names six concrete family
// roots: Decl, Expr, Stmt, Type, Modifier, and Val. The
// cross-cutting consequence is that every well-formed Slang
// program touches multiple families in concert. This test
// instantiates at least one concrete leaf per documented family:
//   - Decl:     FuncDecl (compute), VarDecl (n), GenericDecl
//               (via generic addOne<T>)
//   - Expr:     literal, binary InvokeExpr (n + 1),
//               IntrinsicCallExpr (addOne(...))
//   - Stmt:     BlockStmt, ReturnStmt, DeclStmt
//   - Type:     int (BasicType), generic Type parameter T (DeclRefType)
//   - Modifier: [ForceInline] attribute on addOne
//   - Val:      int literal 5 (IntVal carried as a generic-or-runtime value)
// A single INTERPRET observation verifies they compose into a
// working computation.

//TEST:INTERPRET(filecheck=CHECK):

[ForceInline]
T addOne<T : __BuiltinIntegerType>(T x)
{
    return x + T(1);
}

void main()
{
    int n = 5;
    int r = addOne<int>(n);
    // CHECK: r=6
    printf("r=%d\n", r);
}
