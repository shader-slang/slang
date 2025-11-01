//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -llvm -compute
struct A
{
    uint64_t f0;
    uint32_t f1;
    // Intentional trailing padding here.
};

struct B
{
    float3 f;
};

struct C
{
    A a;            // D:  0-16 | S:  0-12 | C:  0-16
    uint32_t test1; // D: 16-20 | S: 12-16 | C: 16-20
    B b;            // D: 32-48 | S: 16-28 | C: 20-32
    uint32_t test2; // D: 48-52 | S: 28-32 | C: 31-36
    bool c;         // D: 52-56 | S: 32-36 | C: 36-37
    bool d;         // D: 56-60 | S: 36-40 | C: 37-38
};

// All of these buffers should have the same data, if interpreted correctly.
//TEST_INPUT:set std430Buffer = ubuffer(data=[1 2 3 0 4 0 0 0 5 6 7 0 8 1 1], stride=4)
StructuredBuffer<C, Std430DataLayout> std430Buffer;
//TEST_INPUT:set scalarBuffer = ubuffer(data=[1 2 3 4 5 6 7 8 1 1], stride=4)
StructuredBuffer<C, ScalarDataLayout> scalarBuffer;
//TEST_INPUT:set cBuffer = ubuffer(data=[1 2 3 0 4 5 6 7 8 4294967295], stride=4)
StructuredBuffer<C, CDataLayout> cBuffer;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    C d = std430Buffer[0];
    C s = scalarBuffer[0];
    C c = cBuffer[0];

    // CHECK: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    outputBuffer[0] = d.a.f0 == s.a.f0;
    outputBuffer[1] = d.a.f0 == c.a.f0;
    outputBuffer[2] = s.a.f0 == c.a.f0;

    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    outputBuffer[3] = d.a.f1 == s.a.f1;
    outputBuffer[4] = d.a.f1 == c.a.f1;
    outputBuffer[5] = s.a.f1 == c.a.f1;

    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    outputBuffer[6] = d.test1 == s.test1;
    outputBuffer[7] = d.test1 == c.test1;
    outputBuffer[8] = s.test1 == c.test1;

    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    outputBuffer[9] = all(d.b.f == s.b.f);
    outputBuffer[10] = all(d.b.f == c.b.f);
    outputBuffer[11] = all(s.b.f == c.b.f);

    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    outputBuffer[12] = d.test2 == s.test2;
    outputBuffer[13] = d.test2 == c.test2;
    outputBuffer[14] = s.test2 == c.test2;

    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    outputBuffer[15] = d.c == s.c;
    outputBuffer[16] = d.c == c.c;
    outputBuffer[17] = s.c == c.c;

    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    outputBuffer[18] = d.d == s.d;
    outputBuffer[19] = d.d == c.d;
    outputBuffer[20] = s.d == c.d;
}
