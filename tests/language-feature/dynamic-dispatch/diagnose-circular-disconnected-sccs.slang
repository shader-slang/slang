// Interfaces in different dependency cycles must not contaminate one another.
//
// If IA/IB form one cycle and IC/ID form another, an IA implementation that
// merely stores IC does not create a cycle back to IA and must not be diagnosed.

//TEST:SIMPLE(filecheck=CHECK): -target spirv

[anyValueSize(32)]
interface IA
{
    float a();
}

[anyValueSize(32)]
interface IB
{
    float b();
}

[anyValueSize(32)]
interface IC
{
    float c();
}

[anyValueSize(32)]
interface ID
{
    float d();
}

// CHECK: error[E41003]
// CHECK: AToB
// CHECK: circular reference through interface
// CHECK: IA
struct AToB : IA
{
    IB bVal;
    float a() { return bVal.b(); }
}

// CHECK: error[E41003]
// CHECK: BToA
// CHECK: circular reference through interface
// CHECK: IB
struct BToA : IB
{
    IA aVal;
    float b() { return aVal.a(); }
}

// CHECK: error[E41003]
// CHECK: CToD
// CHECK: circular reference through interface
// CHECK: IC
struct CToD : IC
{
    ID dVal;
    float c() { return dVal.d(); }
}

// CHECK: error[E41003]
// CHECK: DToC
// CHECK: circular reference through interface
// CHECK: ID
struct DToC : ID
{
    IC cVal;
    float d() { return cVal.c(); }
}

// This depends on IC, which is cyclic elsewhere, but IC does not reach back to
// IA. The implementation is not itself cyclic and must not be diagnosed.
// CHECK-NOT: AUsesC
struct AUsesC : IA
{
    IC cVal;
    float a() { return cVal.c(); }
}

IA makeA()
{
    AToB x;
    return x;
}

IB makeB()
{
    BToA x;
    return x;
}

IC makeC()
{
    CToD x;
    return x;
}

ID makeD()
{
    DToC x;
    return x;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IA a = makeA();
    IB b = makeB();
    IC c = makeC();
    ID d = makeD();
    outputBuffer[0] = a.a() + b.b() + c.c() + d.d();
}
