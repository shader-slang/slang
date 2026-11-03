// Multiplying by a dereferenced pointer, as in `3 * *p` or `a * **pp`, must parse as a
// multiplication. The `*` after the left operand is followed by more `*` tokens, so the parser
// looks past the whole run of `*` to tell this apart from a pointer type such as `int**`; the
// rules are documented on `_isPointerTypeSuffix` in slang-parser.cpp. The rows below cover a run
// followed by an operand, by `)`, `>`, `,`, `[` and `>>`, and by `(` after a value or a type.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[5 7], stride=4):name=data
uniform int* data;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):name=slot
uniform int** slot;

int three()
{
    return 3;
}

struct Holder
{
    int k;
}

typealias IntAlias = int;

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

    // The cast, `bit_cast` and `sizeof(int**)` read the run of `*` as a pointer type, because
    // it is followed by `)`, `>` or `,`. The `int**` declarations parse their type with
    // `ParseType` and do not depend on this lookahead.
    *slot = p;
    int** pp = (int**)slot;
    int** pp2 = bit_cast<int**, int**>(pp);

    // CHECK-NEXT: 15
    outputBuffer[4] = a * **pp;

    // CHECK-NEXT: 75
    outputBuffer[5] = **pp * **pp2 * a;

    // CHECK-NEXT: 8
    outputBuffer[6] = sizeof(int**);

    // A run of two or more `*` followed by `(` stays a pointer type when the left operand names
    // a type, so the function-style cast `int**(slot)` still compiles.
    int** pp3 = int**(slot);
    // CHECK-NEXT: 15
    outputBuffer[7] = a * **pp3;

    // A run followed by `[` or `>>` is a pointer type.
    // CHECK-NEXT: 24
    outputBuffer[8] = sizeof(int**[3]);
    // CHECK-NEXT: 8
    outputBuffer[9] = sizeof(Ptr<Ptr<int**>>);

    // Before `(`, a run of `*` after a literal, a variable, a call, an indexed variable or a
    // parenthesized expression is a multiply, because none of them names a type. `*(p + 1)` is
    // 22 here, because the `*q +=` row above updated it.
    int arr[1] = { 3 };
    // CHECK-NEXT: 66
    outputBuffer[10] = 3 * *(p + 1);
    // CHECK-NEXT: 66
    outputBuffer[11] = a * *(p + 1);
    // CHECK-NEXT: 66
    outputBuffer[12] = three() * *(p + 1);
    // CHECK-NEXT: 66
    outputBuffer[13] = arr[0] * *(p + 1);
    // CHECK-NEXT: 66
    outputBuffer[14] = (a + 0) * *(p + 1);

    // A member of a value, through `.` or `->`, is a value too.
    Holder h;
    h.k = 3;
    Holder* ph = (Holder*)data;
    int2 v = int2(3, 0);
    // CHECK-NEXT: 66
    outputBuffer[15] = h.k * *(p + 1);
    // CHECK-NEXT: 66
    outputBuffer[16] = v.x * *(p + 1);
    // CHECK-NEXT: 110
    outputBuffer[17] = ph->k * *(p + 1);

    // CHECK-NEXT: 88
    outputBuffer[18] = sizeof(int) * *(p + 1);

    // A type alias before `**(` is a type, so this is a function-style cast.
    IntAlias** pp4 = IntAlias**(slot);
    // CHECK-NEXT: 15
    outputBuffer[19] = a * **pp4;
}
