//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=bdab7c96cac9a55381381f37622841c41f387f80bb1577daf4c1cc5d5aa46b5d
//META: purpose=Boundary: a user-struct element type in a structured buffer is not wrapped, because the element is already a struct.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: pre-existing user struct as the element type.
// wrapStructuredBuffersOfMatrices only fires when the element is
// a bare matrix — a user struct is already in a struct, so the
// pass is a no-op for this shape.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct UserData {
    float a;
    int b;
};

RWStructuredBuffer<UserData> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    UserData d;
    d.a = float(tid.x);
    d.b = int(tid.x);
    outBuf[tid.x] = d;
}

// CHECK: struct UserData_{{[0-9]+}}
// CHECK: RWStructuredBuffer<UserData_{{[0-9]+}}{{.*}}> outBuf_{{[0-9]+}}
