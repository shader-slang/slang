// The `float` and `half` specializations of generic code that converts a constant `double` to `T`
// fold the conversion and emit no `double`, so they still compile for targets that have no
// 64-bit floating-point type. Arguments that convert equally well to `float` and to `double`,
// such as `int64_t`, `uint64_t` and `half`, convert through `float`, never through `double`
// (shader-slang/slang#14062). Without the fix, `T(0.1l)` reaches `IFloat.__init(float)` through
// an implicit conversion that `-warnings-as-errors 30081` turns into an error. A leaked `double`
// fails the `CHECK-NOT` lines on Metal and aborts emission on WGSL.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute -line-directive-mode none -warnings-as-errors 30081
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute -line-directive-mode none -warnings-as-errors 30081 -DNO_INT64

RWStructuredBuffer<int> intInput;
RWStructuredBuffer<half> halfInput;
#ifndef NO_INT64
RWStructuredBuffer<int64_t> int64Input;
RWStructuredBuffer<uint64_t> uint64Input;
#endif

RWStructuredBuffer<float> floatOutput;
RWStructuredBuffer<half> halfOutput;

T tenth<T : __BuiltinFloatingPointType>() { return T(0.1l); }
T tiny<T : __BuiltinFloatingPointType>() { return T(1e-300l); }
T fromInt<T : __BuiltinFloatingPointType>(int value) { return T(value); }
T fromHalf<T : __BuiltinFloatingPointType>(half value) { return T(value); }
#ifndef NO_INT64
T fromInt64<T : __BuiltinFloatingPointType>(int64_t value) { return T(value); }
T fromUInt64<T : __BuiltinFloatingPointType>(uint64_t value) { return T(value); }
#endif

// Metal prints a `double` literal without a type name, so the folded constants are also checked:
// neither the `double` spelling of 0.1 nor 1e-300 may survive.
// CHECK-NOT: double
// CHECK-NOT: 0.10000000000000001
// CHECK-NOT: e-300
// CHECK: computeMain
// CHECK-NOT: double
// CHECK-NOT: 0.10000000000000001
// CHECK-NOT: e-300
[numthreads(1, 1, 1)]
void computeMain()
{
    floatOutput[0] = tenth<float>();
    floatOutput[1] = tiny<float>();
    floatOutput[2] = fromHalf<float>(halfInput[0]);
    floatOutput[5] = fromInt<float>(intInput[0]);
#ifndef NO_INT64
    floatOutput[3] = fromInt64<float>(int64Input[0]);
    floatOutput[4] = fromUInt64<float>(uint64Input[0]);
#endif

    halfOutput[0] = tenth<half>();
    halfOutput[1] = tiny<half>();
    halfOutput[4] = fromInt<half>(intInput[0]);
#ifndef NO_INT64
    halfOutput[2] = fromInt64<half>(int64Input[0]);
    halfOutput[3] = fromUInt64<half>(uint64Input[0]);
#endif
}
