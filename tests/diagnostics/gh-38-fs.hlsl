//TEST_IGNORE_FILE:

// Companion file to `gh-38-vs.hlsl`

Texture2D overlappingB : register(t0);
//CHECK:      ^^^^^^^^^^^^ explicit binding for parameter 'overlappingB' overlaps with parameter 'overlappingA'

Texture2D conflicting : register(t2);

float4 main() : SV_Target { return 0; }
