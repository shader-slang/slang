//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#switchstmt-casestmt-defaultstmt
//META: doc_section_digest=a1ec0803b8c128d1fa4912560d537bd7689c45ee4daaefd344406cf6fe5b8219
//META: purpose=A CaseStmt is "just a marker" that slices the switch body into per-case blocks; an empty case body followed by another case is a fall-through, observable in the emitted text as the two case labels share the next statement block.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says "each CaseStmt/DefaultStmt is just a marker that the
// IR lowering uses to slice the body into per-case blocks. The case
// labels are not the parents of the statements that fall under them".
// The observable form is fall-through: two empty case markers
// followed by one body. Compiling to C++ shows both case labels
// preserved as ordinary `case` markers.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -stage compute

int classify(int v)
{
    int r = 0;
    switch (v)
    {
    case 1:
    case 2:
        r = 22;
        break;
    case 3:
        r = 33;
        break;
    default:
        r = 99;
        break;
    }
    return r;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=output
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int total = classify(1) + classify(2) + classify(3) + classify(99);
    output[0] = total;
    //CHECK: switch
    //CHECK: case int(1)
    //CHECK: case int(2)
}
