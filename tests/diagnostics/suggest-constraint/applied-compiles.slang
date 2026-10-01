// Issue #13140: each constraint the missing-constraint note suggests for a same-module or imported
// user interface in `../suggest-constraint-for-missing-member.slang` and `imports.slang`, once
// applied, makes the failed member access valid. Each function below is the diagnostic test's
// failing case with the suggested constraint added; this test checks they compile and compute the
// expected values.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

module "suggest-constraint-applied";

import "suggest-constraint-lib";

interface IHasNormal { float3 getNormal(); }
interface IFactory { static int make(); }
interface IGenericStaticFactory { static int makeGen<U>(U u); }
interface IHasStaticSm { static int sm(); }
interface IBase { int baseThing(); }
interface IAlpha { int poly(); }
interface IBeta { int poly(); }
interface IHasAssoc
{
    associatedtype assocThing;
    assocThing getAssoc();
    int measure(assocThing x);
}
interface IContainer<T> { T getItem(); }

float3 valueMember<Vertex>(Vertex v) where Vertex : IHasNormal { return v.getNormal(); }
int staticMember<T>() where T : IFactory { return T.make(); }
int genericStaticRequirement<T>() where T : IGenericStaticFactory { return T.makeGen(1); }
int valueStaticRequirement<T>(T v) where T : IHasStaticSm { return v.sm(); }
int inheritedMemberDirectOnly<T>(T t) where T : IBase { return t.baseThing(); }
int twoMatchesSorted<T>(T t) where T : IAlpha { return t.poly(); }
int twoMatchesSortedBeta<T>(T t) where T : IBeta { return t.poly() + 100; }
int assocTypeThroughValue<T>(T t) where T : IHasAssoc
{
    t.assocThing local = t.getAssoc();
    return t.measure(local);
}
int genericInterfaceSuggested<C>(C c) where C : IContainer<int> { return c.getItem(); }
int publicImport<T>(T t) where T : IPublicThing { return t.publicThing(); }
int publicGenericImport<T>(T t) where T : IPublicBox<int> { return t.publicItem(); }

struct Thing : IHasNormal, IFactory, IGenericStaticFactory, IHasStaticSm, IBase, IAlpha, IBeta, IHasAssoc, IContainer<int>, IPublicThing, IPublicBox<int>
{
    float3 getNormal() { return float3(1, 2, 3); }
    static int make() { return 4; }
    static int makeGen<U>(U u) { return 5; }
    static int sm() { return 6; }
    int baseThing() { return 7; }
    int poly() { return 8; }
    typealias assocThing = int;
    int getAssoc() { return 9; }
    int measure(int x) { return x; }
    int getItem() { return 10; }
    int publicThing() { return 11; }
    int publicItem() { return 12; }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Thing t;
    outputBuffer[0] = int(valueMember(t).z);
    outputBuffer[1] = staticMember<Thing>();
    outputBuffer[2] = genericStaticRequirement<Thing>();
    outputBuffer[3] = valueStaticRequirement(t);
    outputBuffer[4] = inheritedMemberDirectOnly(t);
    outputBuffer[5] = twoMatchesSorted(t);
    outputBuffer[6] = assocTypeThroughValue(t);
    outputBuffer[7] = genericInterfaceSuggested(t);
    outputBuffer[8] = publicImport(t);
    outputBuffer[9] = publicGenericImport(t);
    outputBuffer[10] = twoMatchesSortedBeta(t);
    // CHECK: 3
    // CHECK-NEXT: 4
    // CHECK-NEXT: 5
    // CHECK-NEXT: 6
    // CHECK-NEXT: 7
    // CHECK-NEXT: 8
    // CHECK-NEXT: 9
    // CHECK-NEXT: 10
    // CHECK-NEXT: 11
    // CHECK-NEXT: 12
    // CHECK-NEXT: 108
}
