// Test for GitHub issue #9839: Existential Values Nested in Composite Types
//
// Core scenario: dynamically dispatched interface methods receive structs/arrays
// containing nested existentials. The existential must survive the dispatch
// boundary and dispatch correctly inside the implementation.
//
// Patterns tested:
//   1. Single Container (IFoo field) through IProcessor dispatch
//   2. ArrayContainer (IFoo[4]) through IProcessor dispatch
//   3. Two Containers through IProcessor dispatch (multiple nested existentials)
//   4. Container with two different interface fields (IFoo + IBar) through dispatch
//   5. Chained processors: output of one dispatched call feeds another
//   6. IProcessor taking Container + additional IBar parameter (triple dispatch)

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites -conformance "FooA:IFoo=0" -conformance "FooB:IFoo=1" -conformance "BarX:IBar=0" -conformance "BarY:IBar=1" -conformance "ScaleProcessor:IProcessor=0" -conformance "OffsetProcessor:IProcessor=1" -conformance "ArraySumProcessor:IArrayProcessor=0" -conformance "ArrayMaxProcessor:IArrayProcessor=1" -conformance "DualScaleProcessor:IDualProcessor=0" -conformance "DualAddProcessor:IDualProcessor=1" -conformance "MixedScaleProcessor:IMixedProcessor=0" -conformance "MixedSumProcessor:IMixedProcessor=1" -conformance "WeightedProcessor:IWeightedProcessor=0" -conformance "BiasedProcessor:IWeightedProcessor=1"

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// ============================================================
// Interfaces and implementations
// ============================================================

interface IFoo { float eval(); }

struct FooA : IFoo
{
    float val;
    float eval() { return val; }
}

struct FooB : IFoo
{
    float val;
    float eval() { return val * 2.0; }
}

interface IBar { float weight(); }

struct BarX : IBar
{
    float w;
    float weight() { return w; }
}

struct BarY : IBar
{
    float w;
    float weight() { return w * 0.5; }
}

// ============================================================
// Composite types containing existentials
// ============================================================

struct Container
{
    IFoo item;
    int metadata;
}

struct ArrayContainer
{
    IFoo items[4];
}

struct MixedContainer
{
    IFoo foo;
    IBar bar;
}

// ============================================================
// Pattern 1: Single Container through IProcessor dispatch
// ============================================================

interface IProcessor { float process(Container c); }

struct ScaleProcessor : IProcessor
{
    float factor;
    float process(Container c)
    {
        return c.item.eval() * factor + float(c.metadata);
        // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

struct OffsetProcessor : IProcessor
{
    float offset;
    float process(Container c)
    {
        return c.item.eval() + offset + float(c.metadata);
        // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

// ============================================================
// Pattern 2: ArrayContainer through IArrayProcessor dispatch
// ============================================================

interface IArrayProcessor { float processArray(ArrayContainer ac); }

struct ArraySumProcessor : IArrayProcessor
{
    float dummy;
    float processArray(ArrayContainer ac)
    {
        float sum = 0.0;
        for (int i = 0; i < 4; i++)
            sum += ac.items[i].eval();
        return sum;
        // REPORT-DAG: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

struct ArrayMaxProcessor : IArrayProcessor
{
    float dummy;
    float processArray(ArrayContainer ac)
    {
        float m = ac.items[0].eval();
        for (int i = 1; i < 4; i++)
        {
            float v = ac.items[i].eval();
            if (v > m) m = v;
        }
        return m;
        // REPORT-DAG: ([[# @LINE-4]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
        // REPORT-DAG: ([[# @LINE-8]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

// ============================================================
// Pattern 3: Two Containers through IDualProcessor dispatch
// ============================================================

interface IDualProcessor { float processDual(Container a, Container b); }

struct DualScaleProcessor : IDualProcessor
{
    float dummy;
    float processDual(Container a, Container b)
    {
        return a.item.eval() * b.item.eval();
        // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
        // REPORT-DAG: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

struct DualAddProcessor : IDualProcessor
{
    float dummy;
    float processDual(Container a, Container b)
    {
        return a.item.eval() + b.item.eval();
        // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
        // REPORT-DAG: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

// ============================================================
// Pattern 4: MixedContainer (IFoo + IBar) through IMixedProcessor dispatch
// ============================================================

interface IMixedProcessor { float processMixed(MixedContainer mc); }

struct MixedScaleProcessor : IMixedProcessor
{
    float dummy;
    float processMixed(MixedContainer mc)
    {
        return mc.foo.eval() * mc.bar.weight();
        // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
        // REPORT-DAG: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

struct MixedSumProcessor : IMixedProcessor
{
    float dummy;
    float processMixed(MixedContainer mc)
    {
        return mc.foo.eval() + mc.bar.weight();
        // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
        // REPORT-DAG: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

// ============================================================
// Pattern 5 & 6: Container + additional IBar param through IWeightedProcessor
// (triple dispatch: on processor, on container's IFoo, on the IBar param)
// ============================================================

interface IWeightedProcessor { float processWeighted(Container c, IBar w); }

struct WeightedProcessor : IWeightedProcessor
{
    float dummy;
    float processWeighted(Container c, IBar w)
    {
        return c.item.eval() * w.weight();
        // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
        // REPORT-DAG: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

struct BiasedProcessor : IWeightedProcessor
{
    float dummy;
    float processWeighted(Container c, IBar w)
    {
        return c.item.eval() + w.weight() + 1.0;
        // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
        // REPORT-DAG: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    }
}

// ============================================================
// Type conformances and factory functions
// ============================================================

//TEST_INPUT: type_conformance FooA:IFoo = 0
//TEST_INPUT: type_conformance FooB:IFoo = 1
//TEST_INPUT: type_conformance BarX:IBar = 0
//TEST_INPUT: type_conformance BarY:IBar = 1
//TEST_INPUT: type_conformance ScaleProcessor:IProcessor = 0
//TEST_INPUT: type_conformance OffsetProcessor:IProcessor = 1
//TEST_INPUT: type_conformance ArraySumProcessor:IArrayProcessor = 0
//TEST_INPUT: type_conformance ArrayMaxProcessor:IArrayProcessor = 1
//TEST_INPUT: type_conformance DualScaleProcessor:IDualProcessor = 0
//TEST_INPUT: type_conformance DualAddProcessor:IDualProcessor = 1
//TEST_INPUT: type_conformance MixedScaleProcessor:IMixedProcessor = 0
//TEST_INPUT: type_conformance MixedSumProcessor:IMixedProcessor = 1
//TEST_INPUT: type_conformance WeightedProcessor:IWeightedProcessor = 0
//TEST_INPUT: type_conformance BiasedProcessor:IWeightedProcessor = 1

IFoo createFoo(uint id, float data) { return createDynamicObject<IFoo>(id, data); }
IBar createBar(uint id, float data) { return createDynamicObject<IBar>(id, data); }
IProcessor createProcessor(uint id, float data) { return createDynamicObject<IProcessor>(id, data); }
IArrayProcessor createArrayProcessor(uint id, float data) { return createDynamicObject<IArrayProcessor>(id, data); }
IDualProcessor createDualProcessor(uint id, float data) { return createDynamicObject<IDualProcessor>(id, data); }
IMixedProcessor createMixedProcessor(uint id, float data) { return createDynamicObject<IMixedProcessor>(id, data); }
IWeightedProcessor createWeightedProcessor(uint id, float data) { return createDynamicObject<IWeightedProcessor>(id, data); }

// Dispatch helpers

float runProcessor(IProcessor proc, Container c)
{
    return proc.process(c);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float runArrayProcessor(IArrayProcessor proc, ArrayContainer ac)
{
    return proc.processArray(ac);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float runDualProcessor(IDualProcessor proc, Container a, Container b)
{
    return proc.processDual(a, b);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float runMixedProcessor(IMixedProcessor proc, MixedContainer mc)
{
    return proc.processMixed(mc);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

float runWeightedProcessor(IWeightedProcessor proc, Container c, IBar w)
{
    return proc.processWeighted(c, w);
    // REPORT-DAG: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

// ============================================================
// Entry point
// ============================================================

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // ----------------------------------------------------------
    // Test 1: Single Container through IProcessor dispatch
    // ScaleProcessor(factor=3.0) + FooA(5.0), metadata=7
    // 5.0 * 3.0 + 7.0 = 22.0
    // ----------------------------------------------------------
    {
        Container c;
        c.item = createFoo(uint(id), 5.0);
        c.metadata = 7;
        IProcessor proc = createProcessor(uint(id), 3.0);
        outputBuffer[0] = runProcessor(proc, c);
        // CHECK: 22.0
    }

    // ----------------------------------------------------------
    // Test 2: Single Container, different types
    // OffsetProcessor(offset=10.0) + FooB(4.0), metadata=3
    // 4.0*2.0 + 10.0 + 3.0 = 21.0
    // ----------------------------------------------------------
    {
        Container c;
        c.item = createFoo(uint(id + 1), 4.0);
        c.metadata = 3;
        IProcessor proc = createProcessor(uint(id + 1), 10.0);
        outputBuffer[1] = runProcessor(proc, c);
        // CHECK: 21.0
    }

    // ----------------------------------------------------------
    // Test 3: ArrayContainer through IArrayProcessor (sum)
    // ArraySumProcessor + [FooA(1), FooB(2), FooA(3), FooB(4)]
    // = 1.0 + 4.0 + 3.0 + 8.0 = 16.0
    // ----------------------------------------------------------
    {
        ArrayContainer ac;
        ac.items[0] = createFoo(uint(id), 1.0);
        ac.items[1] = createFoo(uint(id + 1), 2.0);
        ac.items[2] = createFoo(uint(id), 3.0);
        ac.items[3] = createFoo(uint(id + 1), 4.0);
        IArrayProcessor proc = createArrayProcessor(uint(id), 0.0);
        outputBuffer[2] = runArrayProcessor(proc, ac);
        // CHECK: 16.0
    }

    // ----------------------------------------------------------
    // Test 4: ArrayContainer through IArrayProcessor (max)
    // ArrayMaxProcessor + [FooA(1), FooB(2), FooA(3), FooB(4)]
    // values: 1.0, 4.0, 3.0, 8.0 → max = 8.0
    // ----------------------------------------------------------
    {
        ArrayContainer ac;
        ac.items[0] = createFoo(uint(id), 1.0);
        ac.items[1] = createFoo(uint(id + 1), 2.0);
        ac.items[2] = createFoo(uint(id), 3.0);
        ac.items[3] = createFoo(uint(id + 1), 4.0);
        IArrayProcessor proc = createArrayProcessor(uint(id + 1), 0.0);
        outputBuffer[3] = runArrayProcessor(proc, ac);
        // CHECK: 8.0
    }

    // ----------------------------------------------------------
    // Test 5: Two Containers through IDualProcessor (multiply)
    // DualScaleProcessor + FooA(3.0) * FooB(2.0) = 3.0 * 4.0 = 12.0
    // ----------------------------------------------------------
    {
        Container a;
        a.item = createFoo(uint(id), 3.0);
        a.metadata = 0;
        Container b;
        b.item = createFoo(uint(id + 1), 2.0);
        b.metadata = 0;
        IDualProcessor proc = createDualProcessor(uint(id), 0.0);
        outputBuffer[4] = runDualProcessor(proc, a, b);
        // CHECK: 12.0
    }

    // ----------------------------------------------------------
    // Test 6: Two Containers through IDualProcessor (add)
    // DualAddProcessor + FooB(5.0) + FooA(7.0) = 10.0 + 7.0 = 17.0
    // ----------------------------------------------------------
    {
        Container a;
        a.item = createFoo(uint(id + 1), 5.0);
        a.metadata = 0;
        Container b;
        b.item = createFoo(uint(id), 7.0);
        b.metadata = 0;
        IDualProcessor proc = createDualProcessor(uint(id + 1), 0.0);
        outputBuffer[5] = runDualProcessor(proc, a, b);
        // CHECK: 17.0
    }

    // ----------------------------------------------------------
    // Test 7: MixedContainer (IFoo + IBar) through IMixedProcessor (scale)
    // MixedScaleProcessor + FooA(6.0) * BarX(3.0) = 6.0 * 3.0 = 18.0
    // ----------------------------------------------------------
    {
        MixedContainer mc;
        mc.foo = createFoo(uint(id), 6.0);
        mc.bar = createBar(uint(id), 3.0);
        IMixedProcessor proc = createMixedProcessor(uint(id), 0.0);
        outputBuffer[6] = runMixedProcessor(proc, mc);
        // CHECK: 18.0
    }

    // ----------------------------------------------------------
    // Test 8: MixedContainer through IMixedProcessor (sum)
    // MixedSumProcessor + FooB(3.0) + BarY(10.0) = 6.0 + 5.0 = 11.0
    // ----------------------------------------------------------
    {
        MixedContainer mc;
        mc.foo = createFoo(uint(id + 1), 3.0);
        mc.bar = createBar(uint(id + 1), 10.0);
        IMixedProcessor proc = createMixedProcessor(uint(id + 1), 0.0);
        outputBuffer[7] = runMixedProcessor(proc, mc);
        // CHECK: 11.0
    }

    // ----------------------------------------------------------
    // Test 9: Triple dispatch — Container + IBar param through IWeightedProcessor
    // WeightedProcessor + FooB(4.0) * BarX(2.0) = 8.0 * 2.0 = 16.0
    // Dispatches: (1) IWeightedProcessor, (2) Container's IFoo, (3) IBar param
    // ----------------------------------------------------------
    {
        Container c;
        c.item = createFoo(uint(id + 1), 4.0);
        c.metadata = 0;
        IBar w = createBar(uint(id), 2.0);
        IWeightedProcessor proc = createWeightedProcessor(uint(id), 0.0);
        outputBuffer[8] = runWeightedProcessor(proc, c, w);
        // CHECK: 16.0
    }

    // ----------------------------------------------------------
    // Test 10: Triple dispatch, different combo
    // BiasedProcessor + FooA(3.0) + BarY(8.0) + 1.0 = 3.0 + 4.0 + 1.0 = 8.0
    // ----------------------------------------------------------
    {
        Container c;
        c.item = createFoo(uint(id), 3.0);
        c.metadata = 0;
        IBar w = createBar(uint(id + 1), 8.0);
        IWeightedProcessor proc = createWeightedProcessor(uint(id + 1), 0.0);
        outputBuffer[9] = runWeightedProcessor(proc, c, w);
        // CHECK: 8.0
    }

    // ----------------------------------------------------------
    // Test 11: Chained processors — output of one feeds context for another
    // ScaleProcessor(2.0) on FooA(5.0) metadata=0 → 5.0*2.0+0 = 10.0
    // then OffsetProcessor(10.0) on FooA(10.0) metadata=0 → 10.0+10.0+0 = 20.0
    // ----------------------------------------------------------
    {
        Container c1;
        c1.item = createFoo(uint(id), 5.0);
        c1.metadata = 0;
        IProcessor proc1 = createProcessor(uint(id), 2.0);
        float intermediate = runProcessor(proc1, c1);

        Container c2;
        c2.item = createFoo(uint(id), intermediate);
        c2.metadata = 0;
        IProcessor proc2 = createProcessor(uint(id + 1), 10.0);
        outputBuffer[10] = runProcessor(proc2, c2);
        // CHECK: 20.0
    }

    // ----------------------------------------------------------
    // Test 12: Cross pattern — MixedContainer with swapped type combos
    // MixedScaleProcessor + FooB(2.0) * BarY(6.0) = 4.0 * 3.0 = 12.0
    // ----------------------------------------------------------
    {
        MixedContainer mc;
        mc.foo = createFoo(uint(id + 1), 2.0);
        mc.bar = createBar(uint(id + 1), 6.0);
        IMixedProcessor proc = createMixedProcessor(uint(id), 0.0);
        outputBuffer[11] = runMixedProcessor(proc, mc);
        // CHECK: 12.0
    }
}
