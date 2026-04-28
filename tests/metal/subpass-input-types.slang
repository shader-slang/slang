//TEST:SIMPLE(filecheck=METAL): -target metal -stage fragment -entry main

// Test SubpassInput with non-float4 element types.

// METAL-DAG: int4
// METAL-DAG: color(0)
// METAL-DAG: uint4
// METAL-DAG: color(1)
// METAL-DAG: float
// METAL-DAG: color(2)

[[vk::input_attachment_index(0)]] SubpassInput<int4> inputInt;
[[vk::input_attachment_index(1)]] SubpassInput<uint4> inputUint;
[[vk::input_attachment_index(2)]] SubpassInput<float> inputScalar;

struct Output
{
    float4 color : SV_Target3;
};

[shader("fragment")]
Output main()
{
    Output o;
    int4 iv = inputInt.SubpassLoad();
    uint4 uv = inputUint.SubpassLoad();
    float sv = inputScalar.SubpassLoad();
    o.color = float4(iv) + float4(uv) + float4(sv, sv, sv, sv);
    return o;
}
