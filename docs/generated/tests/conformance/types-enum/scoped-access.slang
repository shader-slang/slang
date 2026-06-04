//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T16:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-enum.md#description
//META: doc_section_digest=e19ed1fccf556504153f8dba6968221ac410fe7b67d4058007b45ad9c3710e6f
//META: purpose=Enumerations are scoped: named constants are accessed via `EnumType.NAME` form per the doc's "The named constants of a scoped enumeration are accessed within the enumeration namespace using the `EnumType.ENUM_CONST` form."
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

enum Color { Red, Green, Blue }

int colorIndex(Color c)
{
    if (c == Color.Red) return 100;
    if (c == Color.Green) return 200;
    if (c == Color.Blue) return 300;
    return -1;
}

void main()
{
    int r = colorIndex(Color.Red);
    int g = colorIndex(Color.Green);
    int b = colorIndex(Color.Blue);
    //CHECK: r=100 g=200 b=300
    printf("r=%d g=%d b=%d\n", r, g, b);
}
