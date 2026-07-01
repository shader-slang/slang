//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=Boundary: minimum-dimension vector (vector<float,1>) is typed Vec(Float, 1 : Int) in the IR.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `Vec(elementType, elementCount)` is parameterised by
// element count. The smallest documented elementCount is `1`; a
// `uniform vector<float,1>` surfaces `Vec(Float, 1 : Int)` in the
// dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform vector<float, 1> v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.x;
}

// IR: let {{.*}}%v{{.*}}: Vec(Float, 1 : Int){{.*}}= global_param
