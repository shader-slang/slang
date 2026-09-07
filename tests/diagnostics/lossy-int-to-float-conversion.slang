// Verifies that implicit integer -> float / int64_t -> double conversions which
// may lose precision are diagnosed.
//
// An integer *literal* source has an exact known value, so it is diagnosed by
// default when that value is not representable in the target's mantissa (E30133
// for float's 24 bits, E30135 for double's 53 bits); representable, power-of-two,
// and boundary literals are not. A non-constant source cannot be proven lossy at
// compile time and is pervasive in shader code, so it is diagnosed only under
// -Wpedantic (E30134 for float, E30136 for double), and only when the source
// type is wide enough to actually lose precision. Folded constant *expressions*
// are intentionally not diagnosed: constant folding does not wrap at each typed
// operation, so a wrapping expression's folded value would false-positive.
//
// Each warning emits two rows -- the summary and the detailed span message -- so
// the assertions below appear in matching pairs, and repeat where several source
// lines produce the same warning. The check is exhaustive, so the absence of an
// assertion for the representable / power-of-two / explicit-cast / int32->double
// / constant-expression cases also asserts that they produce no diagnostic.
// See #12929.

// Default: only the provably-lossy literal conversions warn.
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT):
//DEFAULT: integer-to-float conversion loses precision
//DEFAULT: integer-to-float conversion loses precision
//DEFAULT: integer-to-float conversion loses precision
//DEFAULT: 'int' to 'float' loses precision
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
//PEDANTIC: integer-to-float conversion loses precision
//PEDANTIC: 'int' to 'float' loses precision
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
    // Literal not representable in float's 24-bit mantissa -> warns by default.
    float a = 123456789;

    // Negative literal, not representable -> warns by default. Its magnitude
    // (123456789), not the two's-complement bit pattern, drives the check.
    float n = -123456789;

    // Representable negative literal -> never warns (magnitude 123 fits, so the
    // sign handling must not read -123 as a huge unsigned payload).
    float rn = -123;

    // Representable literal -> never warns.
    float b = 123;

    // 2^28: 29 bits but a power of two, so exactly representable -> never warns.
    float c = 268435456;

    // 2^24 exactly: the largest run of representable integers ends here -> never warns.
    float bound = 16777216;

    // Explicit cast -> never warns (not an implicit conversion).
    float d = (float)123456789;

    // Parenthesized literal -> warns by default (parentheses are looked through,
    // so this is the same conversion as the bare literal `a`).
    float pa = (123456789);

    // Constant *expression* (folds, but is not a literal) -> not diagnosed by
    // default, even though 2^24 + 1 is lossy: the folded value is not trusted.
    float ce = 16777216 + 1;

    // Wrapping constant expression: uint32 arithmetic makes 0xffffffff + 2 == 1u,
    // so the runtime value is exact. Not diagnosed (guards against warning on the
    // un-wrapped folded value 0x100000001).
    float w = uint(0xffffffff) + 2;

    // Parser-folded unary operators on a literal fold into the literal node with
    // an un-truncated 64-bit payload, but the converted value is the payload
    // reduced to the source width. `-0xffffffff` (a uint) is 1u, `~0xffffffff` is
    // 0u -- both representable, so neither warns (guards the width-reduction path).
    float um = -0xffffffff;
    float un = ~0xffffffff;
    double ud = -0xffffffff;

    // Non-constant int -> float: only under -Wpedantic.
    float e = i;

    // Literal not representable in double's 53-bit mantissa (2^53 + 1) -> warns by default.
    double f = 9007199254740993;

    // Unsigned 64-bit literal with the high bit set: its signed payload reads as
    // -1, so representability must use the unsigned magnitude. Not representable
    // in double -> warns by default (guards the unsigned-payload path).
    double u = 0xFFFFFFFFFFFFFFFF;

    // 2^53 exactly: representable in double -> never warns (double analogue of `bound`).
    double dbound = 9007199254740992;

    // 2^63 as an unsigned 64-bit literal: a power of two, so exactly representable
    // -> never warns (locks the representable end of the unsigned 64-bit path).
    double ub = 0x8000000000000000;

    // 32-bit int -> double is always exact -> never warns, even under -Wpedantic.
    double g = i;

    // Non-constant int64_t -> double: only under -Wpedantic.
    double h = l;
}
