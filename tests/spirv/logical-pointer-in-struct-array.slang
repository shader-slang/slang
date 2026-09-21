//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O1
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly -Xslang -O1

// Array variant of https://github.com/shader-slang/slang/issues/13206.
//
// A local array of structs, each carrying a logical-storage-class pointer, is indexed by a
// constant and written through the pointer. The array element and its pointer field are projected
// via `OpAccessChain`/`OpLoad` rather than extracted from a loaded value, exercising the
// `getElement` projection path. The check runs at -O1 so the pointer must stay address-based
// through the downstream optimizer (slang-test compiles at -O0 unless a level is given).

struct View<let A : AddressSpace>
{
    Ptr<float, Access.ReadWrite, A> data;
    __subscript(int i) -> float
    {
        get { return data[i]; }
        [nonmutating] set { data[i] = newValue; }
    }
}

groupshared float gsA[64];
groupshared float gsB[64];

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// CHECK: OpEntryPoint
// CHECK-NOT: OpCompositeExtract
// CHECK-NOT: OpCompositeConstruct

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    View<AddressSpace.GroupShared> views[2];
    views[0].data = __getAddress(gsA[0]);
    views[1].data = __getAddress(gsB[0]);
    views[0][0] = 3.0f;
    views[1][0] = 4.0f;
    // Read back from groupshared directly, verifying each element's pointer was projected to the
    // right address (a wrong projected index would surface as a wrong value here).
    // BUF: 3
    outputBuffer[0] = gsA[0];
    // BUF: 4
    outputBuffer[1] = gsB[0];
}
