//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=3a40689b7b11a70fa41fa2b7a2e0d38155c8745b85c7301122699a6541c06875
//META: purpose=lowerEnumType replaces enumerator references with the underlying integer literal in WGSL emit
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The enum's underlying integer value is the only thing that
// survives to emit. The enumerator name 'Green' should not appear
// in the output.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

enum Color { Red = 0, Green = 1, Blue = 2 };

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(Color.Green);
}

// CHECK: outBuf_{{[0-9]+}}{{.*}}i32(1)
