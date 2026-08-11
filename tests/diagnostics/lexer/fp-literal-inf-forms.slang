// Regression test for #11276 (Case 2): `0#INF` was rejected by the lexer
// while `1#INF` and `0.#INF` worked. The HLSL/legacy-MSVC infinity form
// `<digits>#INF` is dispatched through `_maybeLexNumberExponent`, but the
// `'0'` arm of `_lexTokenImpl` had its own switch and only fell through
// to integer-literal lowering — `_maybeLexNumberExponent` was never
// called, so the `#` was left dangling for the parser. The same arm also
// affected `0e<digits>` / `0E<digits>` exponent literals (which lexed as
// `IntegerLiteral` with alphanumeric suffix `e10` instead of as a
// `FloatingPointLiteral`, asymmetric with `1e10` which already worked).
// Restore symmetry on both fronts.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // <digit>#INF infinity form
    outputBuffer[0] = 0#INF;
    outputBuffer[1] = 1#INF;
    outputBuffer[2] = 0.#INF;

    // 0 followed by base-10 exponent — must lex as FloatingPointLiteral and
    // evaluate to 0.0, matching `1e10`'s pre-existing behaviour.
    outputBuffer[3] = 0e10;
    outputBuffer[4] = 0E5;
    outputBuffer[5] = 0e+1;
    outputBuffer[6] = 0e-3;
}

// CHECK: outputBuffer{{.*}}[int(0)] = (1.0f / 0.0f);
// CHECK: outputBuffer{{.*}}[int(1)] = (1.0f / 0.0f);
// CHECK: outputBuffer{{.*}}[int(2)] = (1.0f / 0.0f);
// CHECK: outputBuffer{{.*}}[int(3)] = 0.0f;
// CHECK: outputBuffer{{.*}}[int(4)] = 0.0f;
// CHECK: outputBuffer{{.*}}[int(5)] = 0.0f;
// CHECK: outputBuffer{{.*}}[int(6)] = 0.0f;
