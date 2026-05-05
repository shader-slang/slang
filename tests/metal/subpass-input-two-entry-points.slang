//TEST:SIMPLE(filecheck=CHECK): -target metal -entry frag1 -stage fragment -entry frag2 -stage fragment

// CHECK-DAG: frag1{{.*}}color(0)
// CHECK-DAG: frag2{{.*}}color(1)

[[vk::input_attachment_index(0)]] SubpassInput<float4> a;
[[vk::input_attachment_index(1)]] SubpassInput<float4> b;

[shader("fragment")]
float4 frag1() : SV_Target { return a.SubpassLoad(); }

[shader("fragment")]
float4 frag2() : SV_Target { return b.SubpassLoad(); }
