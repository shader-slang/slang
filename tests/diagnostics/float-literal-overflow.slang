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
    // max: 65504, epsilon = 32, limit: 65520
    getType(65519.5hf);
    getType(65520.5hf);
//CHECK:    ^^^^^^^^^ floating-point literal unrepresentable
//CHECK:    ^^^^^^^^^ half literal '65520.5hf' unrepresentable, converted to 'inf'

    // min denormal: ~.0000000596046447
    // half of that: ~.0000000298023223, anything below rounds to zero
    getType(.0000000299hf);
    getType(.0000000297hf);
//CHECK:    ^^^^^^^^^^^^^ floating-point literal too small
//CHECK:    ^^^^^^^^^^^^^ '.0000000297hf' is smaller than the smallest representable value for type half, converted to '0'

    // float overflows and underflows
    // max: ~3.40282346e38, epsilon = .00000020e38, limit: ~3.40282357e38
    getType(3.4028235e38);
    getType(3.4028236e38);
//CHECK:    ^^^^^^^^^^^^ floating-point literal unrepresentable
//CHECK:    ^^^^^^^^^^^^ float literal '3.4028236e38' unrepresentable, converted to 'inf'

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
}
