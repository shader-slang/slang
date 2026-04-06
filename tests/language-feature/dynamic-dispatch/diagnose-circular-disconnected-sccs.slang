// Interfaces in different dependency cycles must not contaminate one another.
//
// If IA/IB form one cycle and IC/ID form another, an IA implementation that
// merely stores IC does not create a cycle back to IA and must not be diagnosed.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl
//TEST:SIMPLE(filecheck=CHECK): -target glsl
//TEST:SIMPLE(filecheck=CHECK): -target cuda
//TEST:SIMPLE(filecheck=CHECK): -target cpp
//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK): -target wgsl
//TEST:SIMPLE(filecheck=CHECK_NEG): -target spirv
//TEST:SIMPLE(filecheck=CHECK_NEG): -target hlsl

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

// CHECK-DAG: error[E41003]
// CHECK-DAG: AToB
// CHECK-DAG: circular reference through interface
// CHECK-DAG: IA
struct AToB : IA
{
    IB bVal;
    float a() { return bVal.b(); }
}

// CHECK-DAG: error[E41003]
// CHECK-DAG: BToA
// CHECK-DAG: circular reference through interface
// CHECK-DAG: IB
struct BToA : IB
{
    IA aVal;
    float b() { return aVal.a(); }
}

// CHECK-DAG: error[E41003]
// CHECK-DAG: CToD
// CHECK-DAG: circular reference through interface
// CHECK-DAG: IC
struct CToD : IC
{
    ID dVal;
    float c() { return dVal.d(); }
}

// CHECK-DAG: error[E41003]
// CHECK-DAG: DToC
// CHECK-DAG: circular reference through interface
// CHECK-DAG: ID
struct DToC : ID
{
    IC cVal;
    float d() { return cVal.c(); }
}

// This depends on IC, which is cyclic elsewhere, but IC does not reach back to
// IA. The implementation is not itself circular and must not get E41003 merely
// because IC is cyclic in a different SCC.
// CHECK_NEG-NOT: type 'AUsesC' contains circular reference through interface 'IA'
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

IA makeAUsingC()
{
    AUsesC x;
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
    IA aFromC = makeAUsingC();
    outputBuffer[0] = a.a() + b.b() + c.c() + d.d() + aFromC.a();
}
