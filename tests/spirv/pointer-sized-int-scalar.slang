// Regression for shader-slang/slang#8480. A fully constant usize_t folds away, so the loop bound is
// taken from the dispatch-thread id to keep a runtime pointer-sized (UIntPtr) value alive through to
// SPIR-V emission, where it is emitted as a 64-bit integer (requiring the Int64 capability).
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// CHECK-DAG: OpCapability Int64{{$}}
// CHECK-DAG: OpTypeInt 64 0

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 tid: SV_DispatchThreadID, uniform RWStructuredBuffer<float> outp)
{
    float acc = 0.0f;

    usize_t count = tid.x;
    for (usize_t i = 0; i < count; i++)
    {
        acc = acc + (float)i;
    }

    outp[0] = acc;
}
