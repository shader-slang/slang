//DIAGNOSTIC_TEST:SIMPLE: -profile sm_5_0 -entry main -stage vertex tests/diagnostics/gh-38-fs.hlsl -entry main -stage fragment -no-codegen

// Ensure that we catch errors with overlapping or conflicting parameter bindings.

Texture2D overlappingA : register(t0);

Texture2D conflicting : register(t1);

float4 main() : SV_Position { return 0; }
