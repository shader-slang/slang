//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

// A negative default for an unsigned generic value parameter folds through the
// parameter's declared type — the same coercion an explicitly written argument
// gets — so the constant wraps like a C conversion (uint16_t(-1) == 65535)
// instead of keeping the unsuffixed literal's own signed type. The implicit
// default and the explicit spelling must denote the identical array type.

struct A<uint16_t x = -1>
{
    int pad[x];
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    A a;      // implicit default: x folds to uint16_t 65535
    A<65535> e; // explicit spelling
    uint32_t szA = uint32_t(sizeof(a.pad) / sizeof(int));
    uint32_t szE = uint32_t(sizeof(e.pad) / sizeof(int));
    outputBuffer[0] = int(szA == 65535) * 2 + int(szE == 65535);
    // CHECK: 3
}
