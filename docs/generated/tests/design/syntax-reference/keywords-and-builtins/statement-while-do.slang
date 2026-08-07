//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=1fb3516e49ddbb5d4259751275e6b594d9d69aa0b1266c092e549ec5de2b449a
//META: purpose=`while` and `do` are statement keywords parsing the head-tested and tail-tested loop forms respectively.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that both loop forms are accepted. The `do { ... } while (c)`
// shape exercises the same `do` keyword that also opens the
// `do { ... } catch ( ... )` statement-level handler described in the
// doc.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int w = 0;
    int j = 0;
    while (j < 3)
    {
        w = w + 2;
        j = j + 1;
    }
    // CHECK: w=6
    printf("w=%d\n", w);

    int d = 0;
    int k = 0;
    do
    {
        d = d + k;
        k = k + 1;
    } while (k < 4);
    // 0 + 1 + 2 + 3 = 6
    // CHECK: d=6
    printf("d=%d\n", d);
}
