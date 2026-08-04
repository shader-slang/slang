//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=49a7ad484e2dfba8ca655298b70ff54f8a19782584f47ca1a503b190c156a5cf
//META: purpose=`TypedefDecl` parses as `'typedef' Type Declarator ';'` (C-style order: type before the alias declarator).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the C-style TypedefDecl production with the simplest
// declarator (a bare IDENT): `typedef Type Name;`. The order is
// reversed from `typealias` — type first, alias declarator second —
// and the typedef'd name is usable as a type.

//TEST:INTERPRET(filecheck=CHECK):

typedef int Distance;

void main()
{
    Distance d = 7;
    // CHECK: d=7
    printf("d=%d\n", d);
}
