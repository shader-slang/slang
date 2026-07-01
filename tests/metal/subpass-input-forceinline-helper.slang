//TEST:SIMPLE(filecheck=METAL): -target metal -stage fragment -entry main

// SubpassInput referenced from [ForceInline] helpers (with deeper indirection)
// must compile and emit [[color(0)]] on the entry-point parameter.

// METAL-DAG: color(0)

[[vk::input_attachment_index(0)]] SubpassInput<float4> fbIn;

[ForceInline]
float4 helperB() { return fbIn.SubpassLoad(); }

[ForceInline]
float4 helperA() { return helperB(); }

[shader("fragment")]
float4 main() : SV_Target { return helperA(); }
