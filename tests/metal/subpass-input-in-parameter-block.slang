//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -stage fragment -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED): -target metal -stage fragment -entry nestedMain

// CHECK: error[E56109]
// CHECK: SubpassInput

struct MyParams
{
    [[vk::input_attachment_index(0)]] SubpassInput<float4> input0;
};

ParameterBlock<MyParams> params;

[shader("fragment")]
float4 main() : SV_Target
{
    return params.input0.SubpassLoad();
}

// Nested struct case.
// NESTED: error[E56109]

struct Inner { [[vk::input_attachment_index(0)]] SubpassInput<float4> fb; };
struct Outer { Inner inner; };
ParameterBlock<Outer> nestedParams;

[shader("fragment")]
float4 nestedMain() : SV_Target
{
    return nestedParams.inner.fb.SubpassLoad();
}
