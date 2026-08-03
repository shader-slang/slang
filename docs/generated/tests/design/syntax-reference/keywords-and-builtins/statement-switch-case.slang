//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=4e5c787f52095c6dbdd20c8c0f4df226f6e2e9bbef0e89d6a888ac6bb5940443
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
