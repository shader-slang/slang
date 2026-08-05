//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=The SourceWriter emits #line N "source" directives in HLSL emit so DXC can map errors back.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The HLSLSourceEmitter goes through the shared SourceWriter,
// which inserts `#line` directives so DXC / fxc diagnostics
// reference the original .slang file.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = 42;
}

// CHECK: #line {{[0-9]+}} "{{.*}}sourcewriter-emits-line-directives.slang"
