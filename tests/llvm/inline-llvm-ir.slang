//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Demonstrate manual return 
// Demonstrate %resut

T reduceAdd<T : __BuiltinFloatingPointType>(vector<T, 4> x)
{
    // $TR = return type of the surrounding function
    // $T0 = Type of first parameter, as SSA value type (aggregates are pointers)
    // $S0 = Type of first parameter, as storage value type (aggregates exist)
    // $0 = first parameter, including type name.
    // $_0 = first parameter, excluding type name.
    __intrinsic_asm "%result = call $TR @llvm.vector.reduce.fadd($TR 0.0, $0)";
    // If the intrinsic asm contains an assignment to %result OR the function
    // returns void, the return instruction is automatically generated such
    // that %result is returned.
}

size_t memset(void* a, uint8_t val, size_t bytes)
{
    __intrinsic_asm R"(
        call void @llvm.memset($0, $1, $2, i1 0)
        ret $2
    )";
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float4 a = float4(1,2,3,4);
    float f1 = reduceAdd(a);
    memset(Ptr<void>(&a), 0, 8);
    float f2 = reduceAdd(a);

    // CHECK: 10.0
    outputBuffer[0] = f1;
    // CHECK-NEXT: 7.0
    outputBuffer[1] = f2;
}
