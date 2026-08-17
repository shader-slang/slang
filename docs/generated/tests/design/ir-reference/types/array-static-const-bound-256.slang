//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#arrays
//META: doc_section_digest=c077a7ede5d6d5b6b14b9cd00c6b8c5162dbde144a6ae57c354c39ace0febb74
//META: purpose=Stress: a large `static const`-bounded array surfaces the full extent as Array(Float, 256 : Int).
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a large `static const`-bounded array surfaces the full extent as
// Array(Float, 256 : Int). A `static const int N = 256;` used as the
// extent is folded to the integer literal `256 : Int` on the `Array`
// opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

static const int N = 256;
struct Big { float data[N]; }
uniform Big b;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = b.data[N - 1];
}

// IR-LABEL: struct %Big
// IR: field(%{{.*}}, Array(Float, 256 : Int))
