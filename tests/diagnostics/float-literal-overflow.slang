//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute
//
// Verifies that float literal overflows and underflows trigger diagnostics. See
// docs/language-reference/expressions-literal.md for details.

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
    // half float overflows and underflows
    // max: 65504, epsilon = 32, limit: <= 65520 (65520 rounds up to 65536)
    getType(65519.999999hf);
    getType(65520.hf);
//CHECK:    ^^^^^^^^ floating-point literal unrepresentable
//CHECK:    ^^^^^^^^ half literal '65520.hf' unrepresentable, converted to 'inf'

    // min denormal: ~.0000000596046447
    // half of that: ~.0000000298023223, anything below rounds to zero
    getType(.0000000299hf);
    getType(.0000000297hf);
//CHECK:    ^^^^^^^^^^^^^ floating-point literal too small
//CHECK:    ^^^^^^^^^^^^^ '.0000000297hf' is smaller than the smallest representable value for type half, converted to '0'

    // float overflows and underflows
    // max: ~3.40282346e38, epsilon = .00000020e38, limit: ~3.40282356e38
    getType(3.40282355e38);
    getType(3.40282357e38);
//CHECK:    ^^^^^^^^^^^^^ floating-point literal unrepresentable
//CHECK:    ^^^^^^^^^^^^^ float literal '3.40282357e38' unrepresentable, converted to 'inf'

    // min denormal: ~1.401298464324e-45
    // half of that: ~.7006492321624e-45, anything below rounds to zero
    getType(.700649233e-45);
    getType(.700649232e-45);
//CHECK:    ^^^^^^^^^^^^^^ floating-point literal too small
//CHECK:    ^^^^^^^^^^^^^^ '.700649232e-45' is smaller than the smallest representable value for type float, converted to '0'

    // double overflows and underflows
    // max: 1.79769313486231570e308, epsilon: ~.000000000000000199e308, limit: 1.79769313486231581e308
    getType(1.79769313486231580e308LF);
    getType(1.79769313486231582e308LF);
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^^^^ floating-point literal unrepresentable
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^^^^ double literal '1.79769313486231582e308LF' unrepresentable, converted to 'inf'

    // min denormal: ~4.94e-324
    // half of that: ~2.47e-324, anything below rounds to zero
    getType(2.48e-324LF);
    getType(2.46e-324LF);
//CHECK:    ^^^^^^^^^^^ floating-point literal too small
//CHECK:    ^^^^^^^^^^^ '2.46e-324LF' is smaller than the smallest representable value for type double, converted to '0'

    // overflowing hex double
    getType(0x1p1023LF);
    getType(0x1p1024LF);
//CHECK:    ^^^^^^^^^^ floating-point literal unrepresentable
//CHECK:    ^^^^^^^^^^ double literal '0x1p1024LF' unrepresentable, converted to 'inf'

    // underflowing hex double
    getType(0x1p-1074LF);
    getType(0x1p-1075LF);
//CHECK:    ^^^^^^^^^^^ floating-point literal too small
//CHECK:    ^^^^^^^^^^^ '0x1p-1075LF' is smaller than the smallest representable value for type double, converted to '0'

    // precision loss (hex double)
    getType(0x1.fffffffffffffp0LF);
    getType(0x1.fffffffffffff8p0LF);
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ floating-point precision lost
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^ significand of '0x1.fffffffffffff8p0LF' was truncated, value is now '0x1.fffffffffffffp+0'

    // precision loss (hex float)
    getType(0x1.fffffep0);
    getType(0x1.ffffffp0);
//CHECK:    ^^^^^^^^^^^^ floating-point precision lost
//CHECK:    ^^^^^^^^^^^^ significand of '0x1.ffffffp0' was truncated, value is now '0x1.fffffep+0'

    // precision loss (hex half)
    getType(0x1.ffcp0hf);
    getType(0x1.ffdp0hf);
//CHECK:    ^^^^^^^^^^^ floating-point precision lost
//CHECK:    ^^^^^^^^^^^ significand of '0x1.ffdp0hf' was truncated, value is now '0x1.ffcp+0'

    // precision loss (hex double, subnormal)
    // subnormals step by 2^-1074; 0x3p-1074 is exact, 0x3.8p-1074 is not
    getType(0x3p-1074LF);
    getType(0x3.8p-1074LF);
//CHECK:    ^^^^^^^^^^^^^ floating-point precision lost
//CHECK:    ^^^^^^^^^^^^^ significand of '0x3.8p-1074LF' was truncated, value is now '0x1.8p-1073'

    // precision loss (hex float, subnormal)
    // subnormals step by 2^-149; 0x3p-149 is exact, 0x3.8p-149 is not
    getType(0x3p-149);
    getType(0x3.8p-149);
//CHECK:    ^^^^^^^^^^ floating-point precision lost
//CHECK:    ^^^^^^^^^^ significand of '0x3.8p-149' was truncated, value is now '0x1.8p-148'

    // precision loss (hex half, subnormal)
    // subnormals step by 2^-24; 0x3p-24 is exact, 0x3.8p-24 is not
    getType(0x3p-24hf);
    getType(0x3.8p-24hf);
//CHECK:    ^^^^^^^^^^^ floating-point precision lost
//CHECK:    ^^^^^^^^^^^ significand of '0x3.8p-24hf' was truncated, value is now '0x1.8p-23'

}
