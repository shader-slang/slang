// user-attribute-float-constant-args.slang

// Tests that floating-point arguments of user-defined attributes reflect their value when the
// argument is a compile-time constant expression rather than a bare literal (issue #13952).

//TEST:REFLECTION(filecheck=CHECK):-stage compute -entry main -target hlsl -no-codegen

[__AttributeUsage(_AttributeTargets.Function)]
struct floatProbeAttribute
{
    int index;
    float value;
};

[__AttributeUsage(_AttributeTargets.Function)]
struct intProbeAttribute
{
    int index;
    int value;
};

[__AttributeUsage(_AttributeTargets.Function)]
struct doubleProbeAttribute
{
    int index;
    double value;
};

[__AttributeUsage(_AttributeTargets.Function)]
struct halfProbeAttribute
{
    int index;
    half value;
};

static const float FLOAT_VALUE = 1.5;
static const int INT_VALUE = 6;
static const float DERIVED_VALUE = FLOAT_VALUE * 2.0 + 0.25;
static float NON_CONST_VALUE = 1.0;

struct Constants
{
    static const float kScale = 4.0;
};

struct Scaled<let N : int>
{
    static const float kValue = N * 0.5;
};

// Specialization and `extern` constants get their values later, so their initializers are only
// defaults and must not be reflected as values.
[SpecializationConstant]
const float SPEC_VALUE = 2.0;
extern static const float EXTERN_VALUE = 3.0;

// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}0,{{$}}
// CHECK-NEXT: {{^ *}}1.500000{{$}}
[floatProbe(0, 1.5)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}1,{{$}}
// CHECK-NEXT: {{^ *}}0.500000{{$}}
[floatProbe(1, 1.0 / 2.0)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}2,{{$}}
// CHECK-NEXT: {{^ *}}1.500000{{$}}
[floatProbe(2, FLOAT_VALUE)]
// CHECK: "name": "intProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}3,{{$}}
// CHECK-NEXT: {{^ *}}3{{$}}
[intProbe(3, INT_VALUE / 2)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}4,{{$}}
// CHECK-NEXT: {{^ *}}-1.500000{{$}}
[floatProbe(4, -(1.5))]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}5,{{$}}
// CHECK-NEXT: {{^ *}}-1.500000{{$}}
[floatProbe(5, -FLOAT_VALUE)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}6,{{$}}
// CHECK-NEXT: {{^ *}}2.000000{{$}}
[floatProbe(6, float(2))]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}7,{{$}}
// CHECK-NEXT: {{^ *}}6.000000{{$}}
[floatProbe(7, INT_VALUE)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}8,{{$}}
// CHECK-NEXT: {{^ *}}3.000000{{$}}
[floatProbe(8, 1.0f * 3.0f)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}9,{{$}}
// CHECK-NEXT: {{^ *}}3.250000{{$}}
[floatProbe(9, DERIVED_VALUE)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}10,{{$}}
// CHECK-NEXT: {{^ *}}2.500000{{$}}
// `INT_VALUE / 4` divides as integers (6 / 4 == 1) before the conversion to `float`.
[floatProbe(10, INT_VALUE / 4 + 1.5 * (2 - 1))]
// CHECK: "name": "doubleProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}11,{{$}}
// CHECK-NEXT: {{^ *}}0.250000{{$}}
[doubleProbe(11, 0.25)]
// CHECK: "name": "halfProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}12,{{$}}
// CHECK-NEXT: {{^ *}}0.500000{{$}}
[halfProbe(12, 0.5)]
// A non-`const` global is not a compile-time constant, so it has no reflected value.
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}13,{{$}}
// CHECK-NEXT: "invalid value"
[floatProbe(13, NON_CONST_VALUE)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}14,{{$}}
// CHECK-NEXT: {{^ *}}4.000000{{$}}
[floatProbe(14, Constants.kScale)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}15,{{$}}
// CHECK-NEXT: {{^ *}}-1.500000{{$}}
[floatProbe(15, -5.5 % 2.0)]
// Each intermediate result rounds to its type: 16777216 + 1 rounds back to 16777216 in `float`,
// so the result is 0, not 1.
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}16,{{$}}
// CHECK-NEXT: {{^ *}}0.000000{{$}}
[floatProbe(16, 16777216.0 + 1.0 - 16777216.0)]
// The `double` counterpart of probe 16 keeps the 1.
// CHECK: "name": "doubleProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}17,{{$}}
// CHECK-NEXT: {{^ *}}1.000000{{$}}
[doubleProbe(17, 16777216.0l + 1.0l - 16777216.0l)]
// 0.1 rounded to `half` is 0.0999755859375.
// CHECK: "name": "halfProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}18,{{$}}
// CHECK-NEXT: {{^ *}}0.099976{{$}}
[halfProbe(18, 0.1)]
// Integer operands wrap to the width of their own type before the conversion.
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}19,{{$}}
// CHECK-NEXT: {{^ *}}4294967296.000000{{$}}
[floatProbe(19, 0u - 1u)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}20,{{$}}
// CHECK-NEXT: {{^ *}}-2147483648.000000{{$}}
[floatProbe(20, 2147483647 + 1)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}21,{{$}}
// CHECK-NEXT: {{^ *}}1.500000{{$}}
[floatProbe(21, +FLOAT_VALUE)]
// 2048 + 1 rounds back to 2048 in `half`, so the result is 0, not 1.
// CHECK: "name": "halfProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}22,{{$}}
// CHECK-NEXT: {{^ *}}0.000000{{$}}
[halfProbe(22, 2048.0h + 1.0h - 2048.0h)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}23,{{$}}
// CHECK-NEXT: "invalid value"
[floatProbe(23, SPEC_VALUE)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}24,{{$}}
// CHECK-NEXT: "invalid value"
[floatProbe(24, EXTERN_VALUE)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}25,{{$}}
// CHECK-NEXT: {{^ *}}1.500000{{$}}
[floatProbe(25, Scaled<3>.kValue)]
// An unsigned 64-bit operand converts as unsigned.
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}26,{{$}}
// CHECK-NEXT: {{^ *}}18446744073709551616.000000{{$}}
[floatProbe(26, 0xFFFFFFFFFFFFFFFFull)]
// Integer operands fold with the integer folder, which does not narrow the intermediate
// `2147483647 + 1` to `int`; a `float` parameter reflects the same value as an `int` one.
// CHECK: "name": "intProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}27,{{$}}
// CHECK-NEXT: {{^ *}}1073741824{{$}}
[intProbe(27, (2147483647 + 1) / 2)]
// CHECK: "name": "floatProbe"
// CHECK-NEXT: "arguments": [
// CHECK-NEXT: {{^ *}}28,{{$}}
// CHECK-NEXT: {{^ *}}1073741824.000000{{$}}
[floatProbe(28, (2147483647 + 1) / 2)]
[numthreads(1, 1, 1)]
void main()
{
}
