// Regression test for #11276 (Case 2): `0#INF` was rejected by the lexer
// while `1#INF` and `0.#INF` worked. The HLSL/legacy-MSVC infinity form
// `<digits>#INF` is dispatched through `_maybeLexNumberExponent`, but the
// `'0'` arm of `_lexTokenImpl` had its own switch and only fell through
// to integer-literal lowering — `_maybeLexNumberExponent` was never
// called, so the `#` was left dangling for the parser. Restore symmetry
// so all three integer prefixes accept `#INF`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = 0#INF;
    outputBuffer[1] = 1#INF;
    outputBuffer[2] = 0.#INF;
}

// CHECK: outputBuffer{{.*}}[int(0)] = (1.0f / 0.0f);
// CHECK: outputBuffer{{.*}}[int(1)] = (1.0f / 0.0f);
// CHECK: outputBuffer{{.*}}[int(2)] = (1.0f / 0.0f);
