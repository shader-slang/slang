//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DUSE_DYN

// tests:
// returning uninitialized val

// CHECK: error 41016

#ifdef USE_DYN
#define QUALIFIER dyn
#else
#define QUALIFIER some
#endif

dyn interface IFoo
{
    int getVal();
}

struct StructForIFoo1 : IFoo
{
    __init(int v)
    {
        a = v;
    }
    int a = 0;
    int getVal()
    {
        return a;
    }

};
struct StructForIFoo2 : IFoo
{
    __init(int v)
    {
        a = v*10;
    }
    int a = 0;
    int getVal()
    {
        return a;
    }

};

QUALIFIER IFoo logic()
{
    QUALIFIER IFoo v1;
    return v1;
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    QUALIFIER IFoo data1 = logic();
    // buffer[0] = data1.getVal();
}