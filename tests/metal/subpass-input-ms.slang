//TEST:SIMPLE(filecheck=METAL): -target metal -stage fragment -entry main
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage fragment -entry main
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG): -target metal -stage fragment -entry main

// Test that SubpassInputMS emits a warning on Metal and still compiles.

// METAL-DAG: color(0)
// METALLIB: framebuffer_fetch
// DIAG: warning[E56105]

[[vk::input_attachment_index(0)]] SubpassInputMS<float4> inputMS;

[shader("fragment")]
float4 main() : SV_Target
{
    return inputMS.SubpassLoad(0);
}
