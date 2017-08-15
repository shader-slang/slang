//TEST:SIMPLE: -target dxbc-assembly -profile vs_5_0 -entry main tests/diagnostics/gh-38-fs.hlsl -profile ps_5_0 -entry main

// Ensure that we catch errors with overlapping or conflicting parameter bindings.

Texture2D overlappingA : register(t0);

Texture2D conflicting : register(t1);

float4 main() : SV_Position { return 0; }
