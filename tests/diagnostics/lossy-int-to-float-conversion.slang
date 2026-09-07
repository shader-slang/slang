// Verifies that implicit integer -> float / int64_t -> double conversions which
// may lose precision are diagnosed.
//
// A constant source is diagnosed by default only when its exact value is not
// representable in the target's mantissa (E30133 for float's 24 bits, E30135
// for double's 53 bits); representable constants, power-of-two constants, and
// explicit casts are not. A non-constant integer source cannot be proven lossy
// at compile time and is pervasive in shader code, so it is only diagnosed
// under -Wpedantic (E30134 for float, E30136 for double), and only when the
// source type is wide enough to actually lose precision.
//
// Each warning emits two rows -- the summary and the detailed span message --
// so the assertions below appear in matching pairs (and twice where two source
// lines produce the same warning). The check is exhaustive, so the absence of
// an assertion for the representable / power-of-two / explicit-cast /
// int32->double cases also asserts that they produce no diagnostic. See #12929.

// Default: only the provably-lossy constant conversions warn.
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT):
//DEFAULT: integer-to-float conversion loses precision
//DEFAULT: integer-to-float conversion loses precision
//DEFAULT: 'int' to 'float' loses precision
//DEFAULT: 'int' to 'float' loses precision
//DEFAULT: integer-to-double conversion loses precision
//DEFAULT: integer-to-double conversion loses precision
//DEFAULT: 'int64_t' to 'double' loses precision
//DEFAULT: 'uint64_t' to 'double' loses precision

// -Wpedantic additionally flags non-constant integer sources wide enough to lose precision.
//DIAGNOSTIC_TEST:SIMPLE(diag=PEDANTIC): -Wpedantic
//PEDANTIC: integer-to-float conversion loses precision
//PEDANTIC: integer-to-float conversion loses precision
//PEDANTIC: 'int' to 'float' loses precision
//PEDANTIC: 'int' to 'float' loses precision
//PEDANTIC: integer-to-float conversion may lose precision
//PEDANTIC: 'int' to 'float' may lose precision
//PEDANTIC: integer-to-double conversion loses precision
//PEDANTIC: integer-to-double conversion loses precision
//PEDANTIC: 'int64_t' to 'double' loses precision
//PEDANTIC: 'uint64_t' to 'double' loses precision
//PEDANTIC: integer-to-double conversion may lose precision
//PEDANTIC: 'int64_t' to 'double' may lose precision

void testConversions(int i, int64_t l)
{
    // Constant literal not representable in float's 24-bit mantissa -> warns by default.
    float a = 123456789;

    // Constant *expression* that folds to a non-representable value (2^24 + 1):
    // exercises the constant-fold path, not just literals -> warns by default.
    float ce = 16777216 + 1;

    // Representable constant -> never warns.
    float b = 123;

    // 2^28: 29 bits but a power of two, so exactly representable -> never warns.
    float c = 268435456;

    // 2^24 exactly: the largest run of representable integers ends here -> never warns.
    float bound = 16777216;

    // Explicit cast -> never warns (not an implicit conversion).
    float d = (float)123456789;

    // Non-constant int -> float: only under -Wpedantic.
    float e = i;

    // Constant not representable in double's 53-bit mantissa (2^53 + 1) -> warns by default.
    double f = 9007199254740993;

    // Unsigned 64-bit constant with the high bit set: its signed bit pattern
    // reads as -1, so representability must use the unsigned magnitude. Not
    // representable in double -> warns by default (guards the signedness path).
    double u = 0xFFFFFFFFFFFFFFFF;

    // 32-bit int -> double is always exact -> never warns, even under -Wpedantic.
    double g = i;

    // Non-constant int64_t -> double: only under -Wpedantic.
    double h = l;
}
