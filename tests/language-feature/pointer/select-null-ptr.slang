// `select` between a pointer and a null pointer of the same type, on the C++ and
// CUDA emitters. A scalar-condition select names its result type, so
// `_slang_select<T>` does not have to deduce `T` from a `nullptr` operand.

//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//CPP: _slang_select<int32_t *>({{.*}}nullptr)
//CPP: _slang_select<void *>({{.*}}nullptr)

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: set dataPtr = ubuffer(data=[10 20], stride=4)
uniform int* dataPtr;

// Runtime flags, so the conditions cannot be folded away.
//TEST_INPUT:ubuffer(data=[1 0], stride=4):name=flagBuffer
RWStructuredBuffer<int> flagBuffer;

int readOr(int* p)
{
    return (p != nullptr) ? *p : -1;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    bool t = flagBuffer[0] != 0;
    bool f = flagBuffer[1] != 0;
    int* p = dataPtr;
    int* null = nullptr;

    // CHECK: 10
    outputBuffer[0] = readOr(select(t, p, (int*)nullptr));
    // CHECK-NEXT: -1
    outputBuffer[1] = readOr(select(f, p, (int*)nullptr));
    // CHECK-NEXT: -1
    outputBuffer[2] = readOr(select(t, null, p + 1));
    // CHECK-NEXT: 20
    outputBuffer[3] = readOr(select(f, null, p + 1));

    void* vp = (void*)p;
    // CHECK-NEXT: 10
    outputBuffer[4] = readOr((int*)select(t, vp, (void*)nullptr));
    // CHECK-NEXT: -1
    outputBuffer[5] = readOr((int*)select(f, vp, (void*)nullptr));
}
