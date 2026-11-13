// Regression for https://github.com/shader-slang/slang/issues/14059: a pointer and
// `nullptr` as the two arms of `?:` (in either order), of `select`, or of a user
// generic `pick<T>(bool, T, T)` infer the pointer type as the common type, and
// `nullptr` converts to it. Each form runs with both a true and a false condition,
// so a pointer arm reads the pointed-to value and a null arm reads -1.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: set dataPtr = ubuffer(data=[10 20], stride=4)
uniform int* dataPtr;

// Runtime flags, so the conditions cannot be folded away.
//TEST_INPUT:ubuffer(data=[1 0 1], stride=4):name=flagBuffer
RWStructuredBuffer<int> flagBuffer;

T pick<T>(bool c, T a, T b)
{
    return c ? a : b;
}

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

    int* n0 = t ? p + flagBuffer[2] : nullptr;
    int* n1 = f ? p + flagBuffer[2] : nullptr;
    // CHECK: 20
    outputBuffer[0] = readOr(n0);
    // CHECK-NEXT: -1
    outputBuffer[1] = readOr(n1);

    // CHECK-NEXT: -1
    outputBuffer[2] = readOr(t ? nullptr : p);
    // CHECK-NEXT: 10
    outputBuffer[3] = readOr(f ? nullptr : p);

    // CHECK-NEXT: 10
    outputBuffer[4] = readOr(select(t, p, nullptr));
    // CHECK-NEXT: -1
    outputBuffer[5] = readOr(select(f, p, nullptr));

    // CHECK-NEXT: 10
    outputBuffer[6] = readOr(pick(t, p, nullptr));
    // CHECK-NEXT: -1
    outputBuffer[7] = readOr(pick(t, nullptr, p));
    // CHECK-NEXT: 10
    outputBuffer[8] = readOr(pick(f, nullptr, p));
    // CHECK-NEXT: -1
    outputBuffer[9] = readOr(pick(f, p, nullptr));

    // CHECK-NEXT: -1
    outputBuffer[10] = readOr(select(t, nullptr, p));
    // CHECK-NEXT: 10
    outputBuffer[11] = readOr(select(f, nullptr, p));
}
