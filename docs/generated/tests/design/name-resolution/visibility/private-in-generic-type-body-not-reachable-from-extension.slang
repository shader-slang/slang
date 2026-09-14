//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#where-visibility-is-filtered
//META: doc_section_digest=bd2e5c9c313f8eee04427c042995baa864cc9e2068805e81fdd68950019c18ec
//META: purpose=The extension target-type alignment is guarded on the candidate's own container being an ExtensionDecl, so a `private` member declared in a generic type's own body is reachable from neither `extension G<int>` nor `extension<T> G<T>` and is rejected with declaration-not-accessible (E30600).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the asymmetry the private-access predicate has between the two
// directions of the extension check. Its mirror image,
// private-in-generic-extension-callable-from-sibling-extension.slang, shows
// that a `private` declared *in an extension* crosses instantiations. Here the
// `private` is declared in `G<T>`'s own body instead, so the candidate's
// container is the generic struct rather than an `ExtensionDecl` and the
// alignment step never runs: the comparison is `G<T>` against the requesting
// extension's target type and it fails. Both spellings of the requesting
// extension are exercised in one file — the concrete `extension G<int>` and
// the generic `extension<T> G<T>` — so the CHECKs prove the rejection is not
// an artefact of one instantiation shape.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct G<T>
{
    private int secret() { return 7; }
}

extension G<int>
{
    int reachFromConcrete() { return secret(); }
//CHECK:                             ^^^^^^ declaration not accessible
//CHECK:                             ^^^^^^ 'secret' is not accessible from the current context.
}

extension<T> G<T>
{
    int reachFromGeneric() { return secret(); }
//CHECK:                            ^^^^^^ declaration not accessible
//CHECK:                            ^^^^^^ 'secret' is not accessible from the current context.
// CHECK: E30600
}

void main()
{
    G<int> g;
    printf("%d\n", g.reachFromConcrete());
}
