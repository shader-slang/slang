// The C++/CUDA emitters name the IR result type in a scalar-condition
// `_slang_select<T>(...)`, so `T` must be spelled correctly for every result type,
// not only pointers. A vector-condition select keeps template deduction.

//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//CPP-DAG: _slang_select<S_0>(
//CPP-DAG: _slang_select<FixedArray<int32_t, 3> >(
//CPP-DAG: _slang_select<Matrix<float, 2, 2> >(
//CPP-DAG: _slang_select<Vector<int32_t, 3> >(
//CPP-DAG: _slang_select<half>(
//CPP-DAG: _slang_select(Vector<bool, 3>

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[1 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct S
{
    int a;
    float b;
}

enum E
{
    A = 3,
    B = 5,
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Read the condition from the buffer so the selects are not folded away.
    bool t = outputBuffer[0] != 0;
    bool f = !t;

    S s0 = { 1, 2.0 };
    S s1 = { 7, 8.0 };
    int arr0[3] = { 1, 2, 3 };
    int arr1[3] = { 4, 5, 6 };
    float2x2 m0 = float2x2(1, 2, 3, 4);
    float2x2 m1 = float2x2(5, 6, 7, 8);
    int3 v0 = int3(1, 2, 3);
    int3 v1 = int3(4, 5, 6);

    // CHECK: 7
    outputBuffer[0] = select(f, s0, s1).a;
    // CHECK-NEXT: 6
    outputBuffer[1] = select(f, arr0, arr1)[2];
    // CHECK-NEXT: 8
    outputBuffer[2] = int(select(f, m0, m1)[1][1]);
    // CHECK-NEXT: 5
    outputBuffer[3] = select(f, v0, v1).y;
    // CHECK-NEXT: 2
    outputBuffer[4] = int(select(t, half(1.5), half(2.5)) + half(0.5));
    // CHECK-NEXT: 1
    outputBuffer[5] = select(t, true, false) ? 1 : 0;
    // CHECK-NEXT: 5
    outputBuffer[6] = int(select(f, E.A, E.B));
    // CHECK-NEXT: 3
    outputBuffer[7] = select(t, E.A, 1);
    // CHECK-NEXT: 5
    outputBuffer[8] = select(bool3(t, f, t), v0, v1).y;
}
