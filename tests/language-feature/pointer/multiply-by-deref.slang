// Multiplying by a dereferenced pointer, as in `3 * *p` or `a * **pp`, must parse as a
// multiplication. The `*` after the left operand is followed by more `*` tokens, so the parser
// looks past the whole run of `*` to tell this apart from a pointer type such as `int**`,
// which is followed by `>`, `)` or `,` instead of an operand.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[5 7], stride=4):name=data
uniform int* data;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):name=slot
uniform int** slot;

[numthreads(1, 1, 1)]
void computeMain()
{
    int* p = data;
    int* q = data + 1;
    int a = 3;

    // CHECK: 15
    outputBuffer[0] = 3 * *p;

    // CHECK-NEXT: 15
    outputBuffer[1] = a * *p;

    // CHECK-NEXT: 22
    *q += 3 * *p;
    outputBuffer[2] = *q;

    // CHECK-NEXT: 330
    outputBuffer[3] = a * *p * *q;

    // The cast, `bit_cast` and `sizeof` arguments read the run of `*` as a pointer type,
    // because it is followed by `)`, `>`, `,`, `[` or `>>`.
    *slot = p;
    int** pp = (int**)slot;
    int** pp2 = bit_cast<int**, int**>(pp);

    // CHECK-NEXT: 15
    outputBuffer[4] = a * **pp;

    // CHECK-NEXT: 75
    outputBuffer[5] = **pp * **pp2 * a;

    // CHECK-NEXT: 8
    outputBuffer[6] = sizeof(int**);

    // A run of two or more `*` followed by `(` stays a pointer type, so the function-style
    // cast `int**(slot)` still compiles.
    int** pp3 = int**(slot);
    // CHECK-NEXT: 15
    outputBuffer[7] = a * **pp3;

    // CHECK-NEXT: 24
    outputBuffer[8] = sizeof(int**[3]);

    // CHECK-NEXT: 8
    outputBuffer[9] = sizeof(Ptr<Ptr<int**>>);
}
