//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#size-and-offset
//META: doc_section_digest=43c0f4ec58392bc558dea4e760d5be974187198677d818ffcac504b279a89de3
//META: purpose=A size attribute puts the LayoutResourceKind in operand 0 and the size in operand 1, so one type layout can carry a separate size per kind.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the kind-first convention on `size`. A parameter block holding both
// byte-sized data and resources gives its struct layout two size attributes;
// the CHECKs capture the first two operands of that struct layout and show
// each is a `size` whose leading operand is a different kind enumerator, which
// is only readable if the kind is operand 0.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct Params
{
    float4 a;
    Texture2D t;
    SamplerState s;
}

ParameterBlock<Params> gPB;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = gPB.a.x;
}

// CHECK: = structTypeLayout(%[[S0:[0-9]+]], %[[S1:[0-9]+]], %{{[0-9]+}}
// CHECK-DAG: %[[S0]]{{.*}}= size(8 : Int, 16 : Int)
// CHECK-DAG: %[[S1]]{{.*}}= size(9 : Int, 2 : Int)
