// Verify BFloat16 handles special floating-point values through conversion.
// Tests NaN, infinity, negative zero, and precision limits.
//
// Note: The bytecode interpreter does not support bf16 arithmetic directly,
// so all special-value tests go through float->bf16->float conversion.
// Arithmetic with special values is exercised by the GPU tests (bf16-arithmetic.slang).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // NaN survives roundtrip through bf16
    float fnan = 0.0 / 0.0;
    BFloat16 bnan = BFloat16(fnan);
    float nan_rt = float(bnan);
    // CHECK: nan_roundtrip=1
    printf("nan_roundtrip=%d\n", int(nan_rt != nan_rt));

    // Positive infinity survives roundtrip
    float finf = 1.0 / 0.0;
    BFloat16 binf = BFloat16(finf);
    float inf_rt = float(binf);
    // CHECK: posinf_roundtrip=1
    printf("posinf_roundtrip=%d\n", int(inf_rt > 1e30));

    // Negative infinity survives roundtrip
    float fninf = -1.0 / 0.0;
    BFloat16 bninf = BFloat16(fninf);
    float ninf_rt = float(bninf);
    // CHECK: neginf_roundtrip=1
    printf("neginf_roundtrip=%d\n", int(ninf_rt < -1e30));

    // Negative zero: bf16(-0.0) == bf16(0.0) (IEEE 754)
    BFloat16 negZero = BFloat16(-0.0);
    BFloat16 posZero = BFloat16(0.0);
    // CHECK: negzero_eq=1
    printf("negzero_eq=%d\n", int(float(negZero) == float(posZero)));

    // BFloat16 precision: 7-bit mantissa, ~2 decimal digits.
    // 256.0 converts exactly (it's a power of 2).
    BFloat16 b256 = BFloat16(256.0);
    // CHECK: exact_256=256
    printf("exact_256=%d\n", int(float(b256)));

    // 257.0 rounds to 256.0 in bf16 (1.0 is below the ULP at 256.0).
    BFloat16 b257 = BFloat16(257.0);
    // CHECK: round_257=256
    printf("round_257=%d\n", int(float(b257)));

    // 258.0 is exactly representable (it's 256 + 2, and ULP at 256 is 2).
    BFloat16 b258 = BFloat16(258.0);
    // CHECK: exact_258=258
    printf("exact_258=%d\n", int(float(b258)));

    // Small values: 0.001953125 = 2^-9, exactly representable.
    BFloat16 bsmall = BFloat16(0.001953125);
    // CHECK: small_exact=1
    printf("small_exact=%d\n", int(float(bsmall) == 0.001953125));

    // Conversion from int
    BFloat16 bint = BFloat16(42);
    // CHECK: from_int=42
    printf("from_int=%d\n", int(float(bint)));

    // Conversion from half
    BFloat16 bhalf = BFloat16(half(7.0));
    // CHECK: from_half=7
    printf("from_half=%d\n", int(float(bhalf)));
}
