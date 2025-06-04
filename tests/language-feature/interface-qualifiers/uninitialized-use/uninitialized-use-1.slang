//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DUSE_DYN

// tests:
// uninitialized `out` assigned to `out`

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

void logic(out QUALIFIER IFoo v1, out QUALIFIER IFoo v2)
{
// CHECK: error 41015
    v1 = v2;
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    QUALIFIER IFoo data1;
    QUALIFIER IFoo data2;
    logic(data1, data2);
    // buffer[0] = data1.getVal();
}