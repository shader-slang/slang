// A `Ptr<float2x3>` on CPU and CUDA can point at a local or at a buffer element under the
// column-major default (shader-slang/slang#13405).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=COL):-cpu -compute -output-using-type -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=ROW):-cpu -compute -output-using-type -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=COL):-cuda -compute -output-using-type -xslang -matrix-layout-column-major

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0], stride=4):name=mats
RWStructuredBuffer<float2x3> mats;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct S
{
    float a;
    float2x3 m;
};

struct H
{
    Ptr<float2x3> p;
};

float readPointer(Ptr<float2x3> p)
{
    return (*p)[0][1];
}

void writePointer(Ptr<float2x3> p)
{
    (*p)[1][2] = 60.0;
}

float readStruct(Ptr<S> s)
{
    return s->m[1][0];
}

float viaInout(inout float2x3 m)
{
    return readPointer(&m);
}

float viaField(H h)
{
    return (*h.p)[1][1];
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float2x3 plain = float2x3(1, 2, 3, 4, 5, 6);

    // COL: 2.0
    // ROW: 2.0
    outputBuffer[0] = readPointer(&plain);

    // COL-NEXT: 60.0
    // ROW-NEXT: 60.0
    writePointer(&plain);
    outputBuffer[1] = plain[1][2];

    // COL-NEXT: 4.0
    // ROW-NEXT: 4.0
    S s;
    s.a = 0;
    s.m = plain;
    outputBuffer[2] = readStruct(&s);

    // COL-NEXT: 2.0
    // ROW-NEXT: 2.0
    outputBuffer[3] = viaInout(plain);

    // COL-NEXT: 5.0
    // ROW-NEXT: 5.0
    H h;
    h.p = &plain;
    outputBuffer[4] = viaField(h);

    // A pointer to a buffer element reads the same element as a direct access. The buffer holds
    // [1 2 3 4 5 6], so element [0][1] is 3 when stored column-major and 2 when stored row-major.
    // COL-NEXT: 3.0
    // COL-NEXT: 3.0
    // ROW-NEXT: 2.0
    // ROW-NEXT: 2.0
    outputBuffer[5] = readPointer(&mats[0]);
    outputBuffer[6] = mats[0][0][1];

    // COL-NEXT: 60.0
    // ROW-NEXT: 60.0
    writePointer(&mats[0]);
    outputBuffer[7] = mats[0][1][2];

    // A pointer made from a raw address reads its matrix row-major on these targets, whatever the
    // layout of the memory behind it. Column-major storage holds this matrix as [1 4 2 5 3 6], so
    // the pointer's [0][1] is 4 there, while the buffer's own [0][1] is 2.
    // COL-NEXT: 4.0
    // ROW-NEXT: 2.0
    mats[0] = float2x3(1, 2, 3, 4, 5, 6);
    let address = (uint64_t)&mats[0];
    outputBuffer[8] = (*(float2x3*)address)[0][1];
}
