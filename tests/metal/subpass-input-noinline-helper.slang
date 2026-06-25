//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target metal -stage fragment -entry main

// SubpassInput used only in a [noinline] helper should produce an error
// because the global cannot be resolved to a fragment entry point.

// CHECK: error[E56108]

[[vk::input_attachment_index(0)]] SubpassInput<float4> fbIn;

[noinline]
float4 helper() { return fbIn.SubpassLoad(); }

[shader("fragment")]
float4 main() : SV_Target { return helper(); }
