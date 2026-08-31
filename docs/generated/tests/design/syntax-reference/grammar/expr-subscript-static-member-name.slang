//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#top-level-structure
//META: doc_section_digest=7765d9b15bc3cf94f40f1c43ffc9e275a43cb76b6b512e8bd81fbafd2cf86f06
//META: purpose=`NameOrSubscript` rewrites `__subscript` to the subscript's internal name only when another `::` follows, leaving `MyType::__subscript(i)` unaffected.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both arms of the `__subscript`-as-static-member rule by putting
// them side by side in one file. The rewrite is invisible in a successful
// parse, but each arm resolves to a different name and so fails differently,
// and the two failures are what separate the arms:
//
//   * with the trailing `::`, the path names the subscript's accessor, so
//     lookup reports the declaration's internal name `operator[]` — the
//     keyword was translated;
//   * without it, the keyword kept its ordinary identifier meaning, so lookup
//     reports a missing member literally spelled `__subscript`.
//
// Pinning the *name each message quotes* is therefore the observation; both
// arms would report the same thing if the rewrite were unconditional.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Box
{
    int data[4];
    __subscript(int i) -> int { get { return data[i]; } }
}

void main()
{
    Box b;
    Box::__subscript::get(b, 2);
//CHECK: ^^^^^^^^^^^ error E30100
//CHECK: ^^^^^^^^^^^ non-static member 'operator[]'
    Box::__subscript(2);
//CHECK: ^^^^^^^^^^^ error E30027
//CHECK: ^^^^^^^^^^^ '__subscript' is not a member of 'typeof(Box)'.
}
