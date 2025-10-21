
//TEST:SIMPLE(filecheck=CHECK): -target spirv

void doSomethingInSpirv()
{

}

void doSomethingInSpirvBindlessTexture()
{

}

// If there are more than one cases in a target switch that are belong to one target, among those
// capability sets, one set has to be super set of all the others. Here, spvDeviceGroup is not a super set of
// spvBindlessTextureNV, and the other way around is also not true. So this should trigger an error.
void someFunc()
{
    __target_switch
    {
    case spvDeviceGroup:
        doSomethingInSpirv();
    // CHECK: ([[# @LINE+1]]): error 36120:
    case spvBindlessTextureNV:
        doSomethingInSpirvBindlessTexture();
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    someFunc();
}
