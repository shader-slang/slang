// Three-interface dependency cycle: IA → IB → IC → IA.
//
// This exercises the cross-interface cycle detection with a cycle length > 2,
// validating that hasDependencyPath correctly traces through multiple hops.

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

// CHECK-DAG: error[E41003]
// CHECK-DAG: AImpl
// CHECK-DAG: circular reference through interface
// CHECK-DAG: IA
struct AImpl : IA
{
    IB bField;
    float a() { return bField.b(); }
}

// CHECK-DAG: error[E41003]
// CHECK-DAG: BImpl
// CHECK-DAG: circular reference through interface
// CHECK-DAG: IB
struct BImpl : IB
{
    IC cField;
    float b() { return cField.c(); }
}

// CHECK-DAG: error[E41003]
// CHECK-DAG: CImpl
// CHECK-DAG: circular reference through interface
// CHECK-DAG: IC
struct CImpl : IC
{
    IA aField;
    float c() { return aField.a(); }
}

IA makeA()
{
    AImpl x;
    return x;
}

IB makeB()
{
    BImpl x;
    return x;
}

IC makeC()
{
    CImpl x;
    return x;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IA a = makeA();
    IB b = makeB();
    IC c = makeC();
    outputBuffer[0] = a.a() + b.b() + c.c();
}
