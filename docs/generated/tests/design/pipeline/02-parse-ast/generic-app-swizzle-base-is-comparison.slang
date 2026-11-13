//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:40:00Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#two-stage-parsing
//META: doc_section_digest=9e6cdcd331c77e6ace71db8f4327394855e85e18930b373da3d6bbea094c0c09
//META: purpose=A base whose checked type is a swizzle value such as `uv.y` forces the comparison reading of `<` even though the token after the matching `>` is a generic follower.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the body-stage `<` rule has a value-typed exception: `uv.y < b > (c)`
// has the token shape `name < name > (`, which a syntactic rule would read as a
// generic application, but the base `uv.y` is a value, so the parser must take
// the comparison reading `(uv.y < b) > c`. A generic reading would be a compile
// error, so a clean run printing r=0 (2.0 < 1 is false, false > 3 is false)
// proves the comparison parse.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float2 uv = float2(1.0, 2.0);
    int b = 1;
    int c = 3;
    bool r = uv.y < b > (c);
    //CHECK: r=0
    printf("r=%d\n", r ? 1 : 0);
}
