// Regression test for shader-slang/slang#8125: an empty struct member retained into emitted
// C/C++/CUDA source has `sizeof(struct{}) == 1`, but Slang type-layout gives it size 0. The
// field *after* the empty member then lands 1 byte off from reflection, so a host that binds
// data at the reflected offset feeds the device (or CPU kernel) a value the kernel reads from
// the wrong offset -> CUDA_ERROR_ILLEGAL_ADDRESS / wrong result.
//
// This is a *runtime* layout test: the harness fills the constant buffer at reflected offsets,
// the kernel reads through emitted offsets. If the empty member survives to source emit, the
// trailing field is read from the wrong offset and the CHECK values diverge. The paired CUDA
// filecheck tests (tests/cuda/cuda-empty-struct*.slang) only assert the emit shape as text;
// this one proves the offsets actually agree. All four reads land inside the first 16-byte
// constant-buffer row, so the reflected layout is identical across targets.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

struct EmptyType
{
}

// Direct empty member: `after` is at reflected offset 4 (EmptyType is size 0 in layout).
struct CallData
{
    int before;
    EmptyType e;
    int after;
}

// Nested empty member (locks the recursive `structContainsEmptyField` path): `Mid` is NOT
// empty (it has `value`), so a one-level check on `Nested` would pass `Mid` through and leave
// the empty `e` to survive. `tag` is at reflected offset 8.
struct Mid
{
    int value;
    EmptyType e;
}
struct Nested
{
    int lead;
    Mid m;
    int tag;
}

//TEST_INPUT:cbuffer(data=[11 22 33 0]):name=callData
ConstantBuffer<CallData> callData;

//TEST_INPUT:cbuffer(data=[100 200 300 400]):name=nested
ConstantBuffer<Nested> nested;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = callData.before; // reflected offset 0
    outputBuffer[1] = callData.after;  // reflected offset 4 -- shifts if EmptyType survives
    outputBuffer[2] = nested.m.value;  // reflected offset 4
    outputBuffer[3] = nested.tag;      // reflected offset 8 -- shifts if nested EmptyType survives

    // CHECK: 11
    // CHECK-NEXT: 22
    // CHECK-NEXT: 200
    // CHECK-NEXT: 300
}
