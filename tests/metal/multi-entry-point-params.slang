//TEST:SIMPLE(filecheck=CHECK): -target metal -fvk-use-entrypoint-name
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF1):-metal -slang -compute -output-using-type -Xslang -DMAIN1
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF2):-metal -slang -compute -output-using-type -Xslang -DMAIN2

#ifdef MAIN1
#define main1 computeMain
#endif
#ifdef MAIN2
#define main2 computeMain
#endif

struct FirstStruct {
    uint a;
}

struct SecondStruct {
    uint a;
}

struct ThirdStruct {
    uint a;
}

struct FourthStruct {
    uint a;
}

//TEST_INPUT:ubuffer(data=[2 0 0 0 0 0 0 0], stride=4):name=globalScopeBuffer
StructuredBuffer<uint> globalScopeBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outBuffer
RWStructuredBuffer<uint> outBuffer;

//
// Checks for the following:
//  - Output entry points will only contain parameters that they originally have from the Slang source. 
//  - Binding offset calculation for global params originating from entry point parameters are done per-whole file.
//  - Binding offset for global params in global scope are the same for each generated entry point.
//

//TEST_INPUT:ubuffer(data=[3 0 0 0], stride=4):name=custom
//TEST_INPUT:ubuffer(data=[5 0 0 0], stride=4):name=other
// NOTE: 'factor' is not initialized because slang-test doesn't have that
//       variable for the 'main2' entry point and thus that case will fail.

// CHECK: main1({{.*}}globalScopeBuffer{{.*}}buffer(0)]], FirstStruct{{.*}}buffer(2)]], SecondStruct{{.*}}buffer(3)]], float{{.*}}buffer(4)]])
[shader("compute")]
[numthreads(5, 1, 1)]
void main1(
    uint3 dispatchThreadID: SV_DispatchThreadID,
    RWStructuredBuffer<FirstStruct> custom,
    RWStructuredBuffer<SecondStruct> other,
    ConstantBuffer<float> factor
) {
    uint index = dispatchThreadID.x;
    outBuffer[index] = globalScopeBuffer[index] * custom[index].a * other[index].a + uint(factor);
    // BUF1: 30
}


// CHECK-NOT: FirstStruct
// CHECK-NOT: SecondStruct
// CHECK: main2({{.*}}globalScopeBuffer{{.*}}buffer(0)]], ThirdStruct{{.*}}buffer(5)]], FourthStruct{{.*}}buffer(6)]])
[shader("compute")]
[numthreads(5, 1, 1)]
void main2(
    uint3 dispatchThreadID: SV_DispatchThreadID,
    RWStructuredBuffer<ThirdStruct> custom,
    RWStructuredBuffer<FourthStruct> other,
) {
    uint index = dispatchThreadID.x;
    outBuffer[index] = globalScopeBuffer[index] * custom[index].a * other[index].a;
    // BUF2: 30
}

