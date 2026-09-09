//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#opswitch-case-literals-follow-the-selector-width
//META: doc_section_digest=261553d1527d9ea1e4d05d9bd40bce371d25c59c042c492c960808b79eb5b1ee
//META: purpose=A switch on an int64_t selector emits OpSwitch case literals wide enough for the 64-bit selector (a value above 2^32 survives intact).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the OpSwitch emit path sizes each case literal from the
// selector's integer width. One case value (4294967296 == 2^32) does not fit
// in a single SPIR-V literal word, so a 32-bit literal would truncate it. The
// selector is loaded from a buffer so the switch is not folded away; CHECK
// asserts the Int64 capability and an OpSwitch whose case literal still reads
// 4294967296 next to a small 64-bit case value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;
StructuredBuffer<int64_t> inBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int64_t sel = inBuf[tid.x];
    int r = 0;
    switch (sel)
    {
    case 4294967296l:
        r = 1;
        break;
    case 7l:
        r = 2;
        break;
    default:
        r = 3;
        break;
    }
    outBuf[tid.x] = r;
}

// CHECK-DAG: OpCapability Int64
// CHECK-DAG: OpSwitch %{{[a-zA-Z_0-9]+}} %{{[a-zA-Z_0-9]+}} 4294967296
