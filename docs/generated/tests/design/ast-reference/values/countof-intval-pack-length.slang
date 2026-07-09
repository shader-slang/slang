//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:13Z
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-family
//META: doc_section_digest=8bbf4cb8daa21d5c3d6e035a494748b9ecec4dbd1090f3e9e250c4555daedc9f
//META: purpose=A CountOfIntVal is a compile-time IntVal that produces the element count of a concrete type pack; the integer is usable wherever a compile-time integer is accepted.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `CountOfIntVal` as a SizeOfLikeIntVal that
// produces "compile-time countof (array length)". The fold in
// `tryFoldOrNull` reduces `countof` of a concrete tuple/pack to
// the integer element count; reading that integer at run time
// shows the documented value.

//TEST:INTERPRET(filecheck=CHECK):

int packCount<each T>(expand each T args)
{
    return countof(T);
}

void main()
{
    // The tuple-of-args has three elements; `countof(T)` folds
    // to a ConstantIntVal of 3.
    // CHECK: n=3
    printf("n=%d\n", packCount(1, 2, 3));
}
