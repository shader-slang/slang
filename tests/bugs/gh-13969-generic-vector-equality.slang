// Regression test for https://github.com/shader-slang/slang/issues/13969
//
// Inside a generic whose element type is constrained to a builtin scalar family
// (`T : __BuiltinFloatingPointType`, `__BuiltinIntegerType` or `__BuiltinLogicalType`),
// `vector<T,N> == vector<T,N>` and `!=` compare per component and yield `vector<bool,N>`, the same as
// for a concrete element type.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

__generic<T : __BuiltinFloatingPointType>
struct Component
{
    vector<T, 3> data;
};

struct ComponentMask
{
    bool3 data;
};

// The shape from the issue: a generic user `operator==` whose body compares generic vectors.
__generic<T : __BuiltinFloatingPointType>
ComponentMask operator==(Component<T> a, Component<T> b)
{
    ComponentMask result;
    result.data = a.data == b.data;
    // The comparison is emitted per component, with no `all` reduction.
    // HLSL: result{{.*}}.data{{.*}} = (a{{.*}}.data{{.*}}) == (b{{.*}}.data{{.*}});
    return result;
}

bool3 neqFloat<T : __BuiltinFloatingPointType>(vector<T, 3> a, vector<T, 3> b)
{
    return a != b;
}

bool3 eqInt<T : __BuiltinIntegerType>(vector<T, 3> a, vector<T, 3> b)
{
    return a == b;
}

bool3 neqInt<T : __BuiltinIntegerType>(vector<T, 3> a, vector<T, 3> b)
{
    return a != b;
}

bool3 eqLogical<T : __BuiltinLogicalType>(vector<T, 3> a, vector<T, 3> b)
{
    return a == b;
}

bool3 eqVectorScalar<T : __BuiltinFloatingPointType>(vector<T, 3> a)
{
    return a == T(0);
}

bool3 eqScalarVector<T : __BuiltinFloatingPointType>(vector<T, 3> a)
{
    return T(0) == a;
}

vector<T, 3> selectEq<T : __BuiltinFloatingPointType>(
    vector<T, 3> a,
    vector<T, 3> b,
    vector<T, 3> x,
    vector<T, 3> y)
{
    return select(a == b, x, y);
}

// A scalar reduction of a generic vector comparison is spelled with `all`/`any`, as it is for a
// concrete element type.
bool allEqFloat<T : __BuiltinFloatingPointType>(vector<T, 3> a, vector<T, 3> b)
{
    return all(a == b);
}

bool anyNeqFloat<T : __BuiltinFloatingPointType>(vector<T, 3> a, vector<T, 3> b)
{
    return any(a != b);
}

void store(int base, bool3 v)
{
    outputBuffer[base + 0] = (uint)v[0];
    outputBuffer[base + 1] = (uint)v[1];
    outputBuffer[base + 2] = (uint)v[2];
}

[numthreads(1, 1, 1)]
void computeMain()
{
    let a = vector<float, 3>(.1f, .0f, .3f);
    let b = vector<float, 3>(0.f);

    // Concrete element type, for reference.
    store(0, a == b);
    // CHECK: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0

    Component<float> a2;
    a2.data = a;
    Component<float> b2;
    b2.data = b;
    store(3, (a2 == b2).data);
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0

    store(6, neqFloat<float>(a, b));
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1

    store(9, eqInt<int>(int3(1, 2, 3), int3(1, 5, 3)));
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1

    store(12, neqInt<uint>(uint3(1, 2, 3), uint3(1, 5, 3)));
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0

    store(15, eqLogical<bool>(bool3(true, false, true), bool3(true, true, true)));
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1

    store(18, eqVectorScalar<float>(a));
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0

    store(21, eqScalarVector<float>(a));
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0

    let selected = selectEq<float>(a, b, float3(1), float3(2));
    outputBuffer[24] = (uint)selected[0];
    outputBuffer[25] = (uint)selected[1];
    outputBuffer[26] = (uint)selected[2];
    // CHECK-NEXT: 2
    // CHECK-NEXT: 1
    // CHECK-NEXT: 2

    outputBuffer[27] = (uint)allEqFloat<float>(a, a);
    outputBuffer[28] = (uint)allEqFloat<float>(a, b);
    outputBuffer[29] = (uint)anyNeqFloat<float>(a, b);
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
}
