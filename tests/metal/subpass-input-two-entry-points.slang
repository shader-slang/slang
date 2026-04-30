//TEST:SIMPLE(filecheck=CHECK): -target metal -entry frag1 -stage fragment -entry frag2 -stage fragment
//TEST:SIMPLE(filecheck=SHARED): -target metal -entry frag3 -stage fragment -entry frag4 -stage fragment

// CHECK-DAG: frag1{{.*}}color(0)
// CHECK-DAG: frag2{{.*}}color(1)

// Test separate globals per entry point.
[[vk::input_attachment_index(0)]] SubpassInput<float4> a;
[[vk::input_attachment_index(1)]] SubpassInput<float4> b;

[shader("fragment")]
float4 frag1() : SV_Target { return a.SubpassLoad(); }

[shader("fragment")]
float4 frag2() : SV_Target { return b.SubpassLoad(); }

// Test one shared global across two entry points.
// SHARED-DAG: frag3{{.*}}color(2)
// SHARED-DAG: frag4{{.*}}color(2)

[[vk::input_attachment_index(2)]] SubpassInput<float4> shared;

[shader("fragment")]
float4 frag3() : SV_Target { return shared.SubpassLoad(); }

[shader("fragment")]
float4 frag4() : SV_Target { return shared.SubpassLoad(); }
