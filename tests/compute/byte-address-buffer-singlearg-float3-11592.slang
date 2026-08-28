//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm
//TEST:SIMPLE(filecheck=CHECK_NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o -

// Regression for https://github.com/shader-slang/slang/issues/11545.
//
// The implicit single-argument `LoadAligned<T>`/`StoreAligned` forms forward `T`'s natural
// *alignment* — the largest power of two dividing `T`'s natural stride — not the stride itself.
// A `float3`'s 12-byte stride is not a power of two, so forwarding the stride would make a
// location of 16 fail the "location is a multiple of the alignment" contract (16 % 12 != 0) and
// be spuriously rejected with E41303. The natural alignment is 4, which divides 16, so the call
// is valid and must compile and round-trip the stored value.
//
// The round-tripped components are written to a typed `RWStructuredBuffer<float>` so the output
// renders as decimal floats (a raw `RWByteAddressBuffer` would render as hex bit patterns).

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):name=scratch
RWByteAddressBuffer scratch;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    scratch.StoreAligned<float3>(16, float3(2.0f, 3.0f, 4.0f));
    float3 v = scratch.LoadAligned<float3>(16);

    // CHECK: 2.0
    outputBuffer[0] = v.x;
    // CHECK: 3.0
    outputBuffer[1] = v.y;
    // CHECK: 4.0
    outputBuffer[2] = v.z;
}

// The optimizer forwards the three Float constants into the byte-buffer writes. Their exact
// IEEE-754 bit patterns reach scratch, while the typed output resource receives Float stores.
// CHECK_NVVM: .visible .const .align 8 .b8 SLANG_globalParams[32]
// CHECK_NVVM: .visible .entry computeMain()
// CHECK_NVVM-DAG: mov.u32 {{.*}}, 1073741824
// CHECK_NVVM-DAG: mov.u32 {{.*}}, 1077936128
// CHECK_NVVM-DAG: mov.u32 {{.*}}, 1082130432
// CHECK_NVVM-DAG: st.global.u32
// CHECK_NVVM-DAG: st.global.u32
// CHECK_NVVM-DAG: st.global.u32
// CHECK_NVVM-DAG: st.global.f32
