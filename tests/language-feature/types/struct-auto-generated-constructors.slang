// struct-auto-generated-constructors.slang

// Test auto-generated default and memberwise constructors.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Pair
{
    int first;
    int second;
}

struct Triple
{
    int a;
    int b;
    int c;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Auto-generated default constructor (zero-initialized)
    Pair p0 = Pair();

    // CHECK: 0
    outputBuffer[0] = p0.first + p0.second;

    // Auto-generated memberwise constructor via {} syntax
    Pair p1 = {10, 20};

    // CHECK: 30
    outputBuffer[1] = p1.first + p1.second;

    // Memberwise constructor for a three-field struct
    Triple t1 = {3, 5, 7};

    // CHECK: 15
    outputBuffer[2] = t1.a + t1.b + t1.c;

    // Verify each field is correctly assigned
    Triple t2 = {100, 200, 300};

    // CHECK: 200
    outputBuffer[3] = t2.b;
}
