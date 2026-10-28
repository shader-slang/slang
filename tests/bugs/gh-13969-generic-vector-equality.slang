// Regression test for https://github.com/shader-slang/slang/issues/13969
//
// Inside a generic whose element type is constrained to a builtin scalar family, `vector<T,N> ==
// vector<T,N>` and `!=` compare per component and yield `vector<bool,N>`, the same as for a
// concrete element type.
//
// The #13969 pins are the `Component<T>` `operator==`, `neqFloat`, `eqInt`, `neqInt`, `eqGenericN`,
// `selectEq`, `ternaryEq`, `anyEqFloat` and `allNeqFloat`. Each of them fails, or returns a
// different value, if the comparison is reduced to a single `bool` (`all(a == b)` for `==`,
// `!all(a == b)` for `!=`). The remaining functions are controls that already compared per
// component through overload resolution: `eqLogical` (`vector<T,N>` has no `IComparable`
// conformance for `__BuiltinLogicalType`) and the mixed vector/scalar `eqVectorScalar` and
// `eqScalarVector`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -xslang -Wno-30056
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
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

vector<bool, N> eqGenericN<T : __BuiltinFloatingPointType, let N : int>(
    vector<T, N> a,
    vector<T, N> b)
{
    return a == b;
}

vector<T, 3> selectEq<T : __BuiltinFloatingPointType>(
    vector<T, 3> a,
    vector<T, 3> b,
    vector<T, 3> x,
    vector<T, 3> y)
{
    return select(a == b, x, y);
}

// A vector `?:` is deprecated (warning 30056, suppressed on the compute test lines) and selects per
// component, like `select`.
vector<T, 3> ternaryEq<T : __BuiltinFloatingPointType>(
    vector<T, 3> a,
    vector<T, 3> b,
    vector<T, 3> x,
    vector<T, 3> y)
{
    return (a == b) ? x : y;
}

bool anyEqFloat<T : __BuiltinFloatingPointType>(vector<T, 3> a, vector<T, 3> b)
{
    return any(a == b);
}

bool allNeqFloat<T : __BuiltinFloatingPointType>(vector<T, 3> a, vector<T, 3> b)
{
    return all(a != b);
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

void store(int base, bool3 v)
{
    outputBuffer[base + 0] = (uint)v[0];
    outputBuffer[base + 1] = (uint)v[1];
    outputBuffer[base + 2] = (uint)v[2];
}

void storeVector(int base, float3 v)
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

    store(15, eqGenericN<float, 3>(a, b));
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0

    storeVector(18, selectEq<float>(a, b, float3(1), float3(2)));
    // CHECK-NEXT: 2
    // CHECK-NEXT: 1
    // CHECK-NEXT: 2

    storeVector(21, ternaryEq<float>(a, b, float3(1), float3(2)));
    // CHECK-NEXT: 2
    // CHECK-NEXT: 1
    // CHECK-NEXT: 2

    outputBuffer[24] = (uint)anyEqFloat<float>(a, b);
    outputBuffer[25] = (uint)allNeqFloat<float>(a, b);
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0

    store(26, eqLogical<bool>(bool3(true, false, true), bool3(true, true, true)));
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1

    store(29, eqVectorScalar<float>(a));
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0

    store(32, eqScalarVector<float>(a));
    // CHECK-NEXT: 0
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0
}
