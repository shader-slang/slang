//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=d6e74209f0cfcbd939127caa9bf2791387db9155c2d0b2f0a561349e2e185456
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
