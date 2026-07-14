//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-c-wgsl-legalization-lowering-phi-elimination
//META: doc_section_digest=eda5f65fdaf66849c226ae86c71a3dd2d4db5ad3899105a4ddc6e7ac2d4447df
//META: purpose=legalizeArrayReturnType rewrites array-returning functions into out-parameter form on WGSL
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL forbids array return values. A T[N] foo() becomes a
// function taking a ptr<function, array<T, N>> output parameter.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int[4] makeArr()
{
    int arr[4] = {1, 2, 3, 4};
    return arr;
}

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a[4] = makeArr();
    outBuf[tid.x] = a[1];
}

// CHECK: fn makeArr
// CHECK-SAME: ptr<function, array<i32
