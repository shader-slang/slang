//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -stage fragment -entry main

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
