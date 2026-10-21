//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=c7c97cf6f5a8dc2bac27a37c7292d84bc70e3e1764ddec58018711cf9fd606ce
//META: purpose=The SourceWriter emits #line directives that reference both the user .slang file and the core/HLSL meta-modules when emitted code originates from there.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `#line` names the file that the emitted code's source location is in.
// `exp10` (hlsl.meta.slang) and `concat` (core.meta.slang) keep a Slang body
// on HLSL and are not inlined, so each is emitted as its own function under
// a meta-module `#line`. Only these two calls matter to the meta CHECKs.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    let pair = concat(makeTuple(float(tid.x)), makeTuple(2.0f));
    outBuf[tid.x] = exp10(pair._0) + pair._1;
}

// CHECK-DAG: #line {{[0-9]+}} "{{.*}}line-directive-references-core-meta.slang"
// CHECK-DAG: #line {{[0-9]+}} "{{.*}}core.meta.slang"
// CHECK-DAG: #line {{[0-9]+}} "{{.*}}hlsl.meta.slang"
