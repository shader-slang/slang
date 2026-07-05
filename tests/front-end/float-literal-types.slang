//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute
//
// Verifies that float literals have correct types as specified in the
// summary table at docs/language-reference/expressions-literal.md.

enum FloatLiteralType
{
    F16 = 1,
    F32 = 2,
    F64 = 3,
}

[ForceInline] FloatLiteralType getType(half      i) { return FloatLiteralType.F16;   }
[ForceInline] FloatLiteralType getType(float     i) { return FloatLiteralType.F32;   }
[ForceInline] FloatLiteralType getType(double    i) { return FloatLiteralType.F64;   }

[numthreads(1,1,1)]
void main()
{
    //CHECK: %main = OpFunction

    // DECIMAL SECTION

    // half-floats
    // all form variants, hf suffix
    static_assert(FloatLiteralType.F16 == getType(1e0hf),           "dec, form 1, hf suffix");
    static_assert(FloatLiteralType.F16 == getType(1.hf),            "dec, form 2 no exponent, hf suffix");
    static_assert(FloatLiteralType.F16 == getType(1.e0hf),          "dec, form 2 with exponent, hf suffix");
    static_assert(FloatLiteralType.F16 == getType(1.0hf),           "dec, form 3 no exponent, hf suffix");
    static_assert(FloatLiteralType.F16 == getType(1.0e0hf),         "dec, form 3 with exponent, hf suffix");

    // rest of the half-float suffixes
    static_assert(FloatLiteralType.F16 == getType(1e0h),            "dec, form 1, h suffix");
    static_assert(FloatLiteralType.F16 == getType(1e0H),            "dec, form 1, H suffix");
    static_assert(FloatLiteralType.F16 == getType(1e0HF),           "dec, form 1, HF suffix");
    static_assert(FloatLiteralType.F16 == getType(1e0fh),           "dec, form 1, fh suffix");
    static_assert(FloatLiteralType.F16 == getType(1e0FH),           "dec, form 1, FH suffix");

    // infinity and positive/negative exponents
    static_assert(FloatLiteralType.F16 == getType(1.0e+1hf),        "dec, form 3 with positive exponent, hf suffix");
    static_assert(FloatLiteralType.F16 == getType(1.0e-1hf),        "dec, form 3 with negative exponent, hf suffix");
    static_assert(FloatLiteralType.F16 == getType(1.0#INFhf),       "dec, form 3 with infinity exponent, hf suffix");

    // floats
    // all form variants, no suffix
    static_assert(FloatLiteralType.F32 == getType(1e0),             "dec, form 1, no suffix");
    static_assert(FloatLiteralType.F32 == getType(1.),              "dec, form 2 no exponent, no suffix");
    static_assert(FloatLiteralType.F32 == getType(1.e0),            "dec, form 2 with exponent, no suffix");
    static_assert(FloatLiteralType.F32 == getType(1.0),             "dec, form 3 no exponent, no suffix");
    static_assert(FloatLiteralType.F32 == getType(1.0e0),           "dec, form 3 with exponent, no suffix");

    // rest of the float suffixes
    static_assert(FloatLiteralType.F32 == getType(1e0f),            "dec, form 1, f suffix");
    static_assert(FloatLiteralType.F32 == getType(1e0F),            "dec, form 1, F suffix");

    // infinity and positive/negative exponents
    static_assert(FloatLiteralType.F32 == getType(1.0e+1),          "dec, form 3 with positive exponent, no suffix");
    static_assert(FloatLiteralType.F32 == getType(1.0e-1),          "dec, form 3 with negative exponent, no suffix");
    static_assert(FloatLiteralType.F32 == getType(1.0#INF),         "dec, form 3 with infinity exponent, no suffix");

    // doubles
    // all form variants, hf suffix
    static_assert(FloatLiteralType.F64 == getType(1e0lf),           "dec, form 1, lf suffix");
    static_assert(FloatLiteralType.F64 == getType(1.lf),            "dec, form 2 no exponent, lf suffix");
    static_assert(FloatLiteralType.F64 == getType(1.e0lf),          "dec, form 2 with exponent, lf suffix");
    static_assert(FloatLiteralType.F64 == getType(1.0lf),           "dec, form 3 no exponent, lf suffix");
    static_assert(FloatLiteralType.F64 == getType(1.0e0lf),         "dec, form 3 with exponent, lf suffix");

    // rest of the double suffixes
    static_assert(FloatLiteralType.F64 == getType(1e0l),            "dec, form 1, l suffix");
    static_assert(FloatLiteralType.F64 == getType(1e0L),            "dec, form 1, L suffix");
    static_assert(FloatLiteralType.F64 == getType(1e0LF),           "dec, form 1, LF suffix");
    static_assert(FloatLiteralType.F64 == getType(1e0fl),           "dec, form 1, fl suffix");
    static_assert(FloatLiteralType.F64 == getType(1e0FL),           "dec, form 1, FL suffix");

    // infinity and positive/negative exponents
    static_assert(FloatLiteralType.F64 == getType(1.0e+1lf),        "dec, form 3 with positive exponent, lf suffix");
    static_assert(FloatLiteralType.F64 == getType(1.0e-1lf),        "dec, form 3 with negative exponent, lf suffix");
    static_assert(FloatLiteralType.F64 == getType(1.0#INFlf),       "dec, form 3 with infinity exponent, lf suffix");

    // HEXADECIMAL SECTION

    // half-floats
    // all form variants, hf suffix
    static_assert(FloatLiteralType.F16 == getType(0x1p0hf),         "hex, form 1, hf suffix");
    static_assert(FloatLiteralType.F16 == getType(0x1.p0hf),        "hex, form 2, hf suffix");
    static_assert(FloatLiteralType.F16 == getType(0x1.0p0hf),       "hex, form 3, hf suffix");

    // infinity and positive/negative exponents
    static_assert(FloatLiteralType.F16 == getType(0x1.0p+1hf),      "hex, form 3, hf suffix");
    static_assert(FloatLiteralType.F16 == getType(0x1.0p-1hf),      "hex, form 3, hf suffix");
    static_assert(FloatLiteralType.F16 == getType(0x1.0#INFhf),     "hex, form 3, hf suffix");

    // floats
    // all form variants, hf suffix
    static_assert(FloatLiteralType.F32 == getType(0x1p0),           "hex, form 1, no suffix");
    static_assert(FloatLiteralType.F32 == getType(0x1.p0),          "hex, form 2, no suffix");
    static_assert(FloatLiteralType.F32 == getType(0x1.0p0),         "hex, form 3, no suffix");

    // infinity and positive/negative exponents
    static_assert(FloatLiteralType.F32 == getType(0x1.0p+1),        "hex, form 3, no suffix");
    static_assert(FloatLiteralType.F32 == getType(0x1.0p-1),        "hex, form 3, no suffix");
    static_assert(FloatLiteralType.F32 == getType(0x1.0#INF),       "hex, form 3, no suffix");

    // doubles
    // all form variants, lf suffix
    static_assert(FloatLiteralType.F64 == getType(0x1p0lf),         "hex, form 1, lf suffix");
    static_assert(FloatLiteralType.F64 == getType(0x1.p0lf),        "hex, form 2, lf suffix");
    static_assert(FloatLiteralType.F64 == getType(0x1.0p0lf),       "hex, form 3, lf suffix");

    // infinity and positive/negative exponents
    static_assert(FloatLiteralType.F64 == getType(0x1.0p+1lf),      "hex, form 3, lf suffix");
    static_assert(FloatLiteralType.F64 == getType(0x1.0p-1lf),      "hex, form 3, lf suffix");
    static_assert(FloatLiteralType.F64 == getType(0x1.0#INFlf),     "hex, form 3, lf suffix");
}
