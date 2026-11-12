//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// The core module splats builtin scalar and enum sources into vectors with `__builtin_cast`,
// and splats a one-element vector into a vector of the same element type. These conversions
// work in concrete and generic code, and forward and backward derivatives flow through the
// one-element splat, both implicit and explicit. A floating-point scalar splats into a vector of
// a different floating-point element type, which relies on the floating-point base types
// declaring `__BuiltinScalarType` directly. An explicit `float4(s)` applies a user conversion
// to the element type and then splats.

enum Color : int
{
    Black = 0,
    Red = 3,
    Green = 4,
}

struct S
{
    float x;
}

extension float
{
    __implicit_conversion(500)
    __init(S v) { this = v.x; }
}

[Differentiable]
float sumOfSplat(float1 v)
{
    float4 r = v;
    return r.x + r.y + r.z + r.w;
}

[Differentiable]
float sumOfExplicitSplat(float1 v)
{
    float4 r = float4(v);
    return r.x + r.y + r.z + r.w;
}

vector<T, N> splatGenericOneVector<T : __BuiltinFloatingPointType, let N : int>(vector<T, 1> v)
{
    return v;
}

bool4 splatGenericEnum<E : __EnumType>(E e)
{
    return e;
}

//TEST_INPUT:ubuffer(data=[2 1 0 0], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int i = inputBuffer[0];
    bool b = inputBuffer[1] != 0;
    Color c = inputBuffer[1] != 0 ? Color.Green : Color.Red;
    Color black = inputBuffer[2] != 0 ? Color.Red : Color.Black;

    // CHECK: 2.0
    float4 fromInt = i;
    outputBuffer[0] = fromInt.w;

    // CHECK-NEXT: 1.0
    int4 fromBool = b;
    outputBuffer[1] = float(fromBool.z);

    // CHECK-NEXT: 4.0
    uint4 mask = select(int4(i) == 0, 0, 4u);
    outputBuffer[2] = float(mask.y);

    // CHECK-NEXT: 4.0
    int4 fromEnum = c;
    outputBuffer[3] = float(fromEnum.w);

    // CHECK-NEXT: 1.0
    bool4 boolFromEnum = c;
    outputBuffer[4] = boolFromEnum.y ? 1.0 : 0.0;

    // CHECK-NEXT: 0.0
    bool4 boolFromZeroEnum = black;
    outputBuffer[5] = boolFromZeroEnum.x ? 1.0 : 0.0;

    // CHECK-NEXT: 2.0
    float1 one = float1(float(i));
    float4 fromOneVector = one;
    outputBuffer[6] = fromOneVector.w;

    // CHECK-NEXT: 2.0
    outputBuffer[7] = float4(one).y;

    // CHECK-NEXT: 7.0
    int1 fromUint1 = uint1(7);
    outputBuffer[8] = float(fromUint1.x);

    // CHECK-NEXT: 2.0
    outputBuffer[9] = splatGenericOneVector<float, 3>(one).z;

    // CHECK-NEXT: 1.0
    outputBuffer[10] = splatGenericEnum(c).w ? 1.0 : 0.0;

    // CHECK-NEXT: 2.0
    S s = { float(i) };
    outputBuffer[11] = float4(s).w;

    // CHECK-NEXT: 8.0
    // CHECK-NEXT: 4.0
    var result = fwd_diff(sumOfSplat)(diffPair(one, float1(1.0)));
    outputBuffer[12] = result.p;
    outputBuffer[13] = result.d;

    // CHECK-NEXT: 4.0
    outputBuffer[14] = fwd_diff(sumOfExplicitSplat)(diffPair(one, float1(1.0))).d;

    // CHECK-NEXT: 4.0
    var pair = diffPair(one);
    bwd_diff(sumOfSplat)(pair, 1.0);
    outputBuffer[15] = pair.d.x;

    // CHECK-NEXT: 2.0
    float f = float(i);
    double4 fromFloat = f;
    outputBuffer[16] = float(fromFloat.z);

    // CHECK-NEXT: 2.0
    half h = half(i);
    float4 fromHalf = h;
    outputBuffer[17] = fromHalf.y;
}
