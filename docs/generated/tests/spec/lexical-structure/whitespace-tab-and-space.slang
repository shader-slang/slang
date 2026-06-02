//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/lexical-structure.md#whitespace
//META: doc_section_digest=966a201fda28c775172186dbde06cb14891d584c984af5c90890d8c08ff9e353
//META: purpose=Both space (U+0020) and horizontal tab (U+0009) count as horizontal whitespace per the doc; either may separate tokens.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
	int		a	=	5;	// tabs throughout (this line is intentionally tab-separated)
    int b = 7;
    int sum = a	+	b;
    //CHECK: sum=12
    printf("sum=%d\n", sum);
}
