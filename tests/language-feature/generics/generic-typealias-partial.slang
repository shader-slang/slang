// generic-typealias-partial.slang

// Test generic type alias for partial binding of generic struct parameters.
// A typealias fixes some parameters while leaving others open.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> output;

struct GenStruct<T : IArithmetic, let N : int>
{
    T value;
    T compute() { return value * T(N); }
}

// Partial binding: fix T=int, leave N open
typealias IntGen<let N : int> = GenStruct<int, N>;

// Partial binding: fix T=float, leave N open
typealias FloatGen<let N : int> = GenStruct<float, N>;

[numthreads(1, 1, 1)]
void computeMain()
{
    IntGen<3> a;
    a.value = 5;
    // 5 * 3 = 15
    // CHECK: 15
    output[0] = a.compute();

    IntGen<7> b;
    b.value = 2;
    // 2 * 7 = 14
    // CHECK: 14
    output[1] = b.compute();

    FloatGen<4> c;
    c.value = 2.5;
    // int(2.5 * 4.0) = int(10.0) = 10
    // CHECK: 10
    output[2] = int(c.compute());

    // CHECK: 39
    output[3] = a.compute() + b.compute() + int(c.compute());
}
