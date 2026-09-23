//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#legalizecall-and-the-ptrfunction-t-address-space-rule
//META: doc_section_digest=be15d3c3215e5a9573f5410bea3b67209427e8137b1e2cb1df5c104f9a77fe1a
//META: purpose=A block-local Var passed to an out parameter is handed to the callee directly because it already lives in the function address space
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `Var whose parent is an IRBlock` row of legalizeCall's
// argument table -- the contrast case to the module-scope global. A local
// variable is already in WGSL's `function` address space, so no bridging
// temporary may be introduced. CHECK asserts the call passes the address of
// the user-named local itself, which is only true when the pass left the
// argument untouched.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

void bump(out int v) { v = 5; }

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int local = 0;
    bump(local);
    outBuf[tid.x] = local;
}

// CHECK: fn bump_{{[0-9]+}}( {{[A-Za-z0-9_]+}} : ptr<function, i32>)
// CHECK: var local_{{[0-9]+}} : i32
// CHECK: bump_{{[0-9]+}}(&(local_{{[0-9]+}}))
