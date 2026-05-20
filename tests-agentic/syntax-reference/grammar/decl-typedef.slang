//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=1da21f4604b0cc983d0ee89029633af70ee59186fe9ef2a8dafdd1550c122368
//META: purpose=`TypedefDecl` parses as `'typedef' Type IDENT ';'` (C-style order: type before name).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the C-style TypedefDecl production: `typedef Type Name;`.
// Note the order is reversed from `typealias` — type comes first, name
// second. The typedef'd name is usable as a type.

//TEST:INTERPRET(filecheck=CHECK):

typedef int Distance;

void main()
{
    Distance d = 7;
    // CHECK: d=7
    printf("d=%d\n", d);
}
