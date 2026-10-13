// Regression test for #12104.
//
// -O1 is explicit because slang-test defaults new tests to -O0; the
// `value / value` fold that produces the constant below only runs once the
// SPIR-V optimizer does, i.e. above -O0.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -emit-spirv-directly -O1

StructuredBuffer<float3> input;
RWStructuredBuffer<float3> output;

[numthreads(1, 1, 1)]
void main()
{
    float3 value = input[0];
    // The `{{$}}` anchor pins the vec3 ones-constant to exactly three
    // constituents; the bug emitted a fourth.
    // CHECK: OpConstantComposite %v3float %float_1 %float_1 %float_1{{$}}
    output[0] = select(value < 0.00001f, float3(0.0f), value / value);
}
