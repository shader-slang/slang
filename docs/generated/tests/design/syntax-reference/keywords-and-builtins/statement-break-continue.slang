//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=58adeae3c01b07a208e3d126aee114b032a435add75925b7e18289c39283719b
//META: purpose=`break` exits the enclosing loop and `continue` skips to the next iteration; both are statement keywords.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Iterate 0..9, skip the i==2 iteration with `continue`, and exit on
// i==5 with `break`. The remaining accumulation is
// 0 + 1 + 3 + 4 = 8.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int s = 0;
    for (int i = 0; i < 10; i = i + 1)
    {
        if (i == 2) { continue; }
        if (i == 5) { break; }
        s = s + i;
    }
    // CHECK: s=8
    printf("s=%d\n", s);
}
