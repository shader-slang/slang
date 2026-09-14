//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=58adeae3c01b07a208e3d126aee114b032a435add75925b7e18289c39283719b
//META: purpose=`switch`, `case`, and `default` form the multi-way dispatch statement; the parser recognizes all three as statement keywords.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The interpreter cannot evaluate `switch` bytecode in all
// configurations, so we observe the parse instead by checking that
// the emitted C++ contains a `switch` construct. If the parser
// dropped `case` or `default`, the program would not compile.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

int classify(int v)
{
    switch (v)
    {
    case 1:  return 100;
    case 2:  return 200;
    default: return 999;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    int a = classify(1) + classify(2) + classify(5);
    //CHECK: switch
    printf("a=%d\n", a);
}
