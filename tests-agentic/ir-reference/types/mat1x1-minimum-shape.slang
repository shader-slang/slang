//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=Boundary: minimum-shape matrix (matrix<float,1,1>) is typed Mat(Float, 1 : Int, 1 : Int, layout) in the IR.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `Mat(elementType, rowCount, columnCount, layout)`
// "adds two shape operands and a `layout` int literal". The
// minimum row/col shape of 1x1 surfaces
// `Mat(Float, 1 : Int, 1 : Int, ...)` in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform matrix<float, 1, 1> m;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = m[0][0];
}

// IR: let {{.*}}%m{{.*}}: Mat(Float, 1 : Int, 1 : Int, {{[0-9]+}} : Int){{.*}}= global_param
