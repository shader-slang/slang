//TEST:SIMPLE(filecheck=SPV): -target spirv -O0

struct Bottom
{
    float bigArray[1024];
    // SPV: %Bottom_bottomGetValue = OpFunction %float None %{{.*}}
    // SPV-NEXT: %{{.*}} = OpFunctionParameter %int
    // SPV-NEXT: OpLabel
    // SPV-NOT: OpCompositeConstruct
    // SPV: OpFunctionEnd

    // SPV: %Bottom_bottomGetValue_0 = OpFunction %float None %{{.*}}
    // SPV-NEXT: %{{.*}} = OpFunctionParameter %int
    // SPV-NEXT: OpLabel
    float bottomGetValue(int index) { return bigArray[index]; }
}

struct Root
{
    Bottom bottom1;
    Bottom bottom2;
}

ConstantBuffer<Root> cb;

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = cb.bottom1.bottomGetValue(0);
    outputBuffer[1] = cb.bottom2.bottomGetValue(1);
    outputBuffer[2] = cb.bottom2.bottomGetValue(2);
}
