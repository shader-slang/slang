//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=A struct declared with non-static fields allocates one copy of each field per instance; fields are read and written via the dot operator.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct with two int fields. Each instance owns separate storage for both fields.
// Reading and writing obj.x and obj.y exercises basic field access via the dot operator.

//TEST:INTERPRET(filecheck=CHECK):

struct Point
{
    int x;
    int y;
}

void main()
{
    Point p;
    p.x = 3;
    p.y = 7;
    // CHECK: 3
    // CHECK: 7
    printf("%d\n%d\n", p.x, p.y);
}
