//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=232bb23f0daa18d59b047831901c0b444bf01283e5c09bd1d859fc8824f19b7c
//META: purpose=Boundary: `typedef` is the C-style decl keyword listed alongside `typealias`; the alias resolves to its underlying type for overload resolution just like `typealias`.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pairs with `decl-typealias.slang` to cover both spellings the
// decl-keywords table lists (`typedef` -> `parseTypeDef`,
// `typealias` -> `parseTypeAliasDecl`).

//TEST:INTERPRET(filecheck=CHECK):

typedef int MyInt;

int kind(int v)  { return 1; }
int kind(uint v) { return 2; }

void main()
{
    MyInt x = 99;
    // CHECK: k=1
    printf("k=%d\n", kind(x));
}
