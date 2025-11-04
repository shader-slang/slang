//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: OpEntryPoint

void doSomethingInSpirv()
{

}

void doSomethingInSpirvBindlessTexture()
{

}

void someFunc()
{
    __target_switch
    {
    case spirv:
        doSomethingInSpirv();
    case spvBindlessTextureNV:
        doSomethingInSpirvBindlessTexture();
    }
}

// Previously, we required `testFunc` to declare the `spvBindlessTextureNV` capability, which is wrong, because __target_switch
// is supposed to introduce (spirv | spvBindlessTextureNV) capability, which will result in just `spirv` capability.
[require(spirv)]
void testFunc()
{
    someFunc();
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    testFunc();
}
