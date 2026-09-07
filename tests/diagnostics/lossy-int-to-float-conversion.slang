// Verifies that implicit integer -> float / int64_t -> double conversions which
// may lose precision are diagnosed.
//
// An integer *literal* source has an exact known value, so it is diagnosed by
// default when that value is not representable in the target's significand
// (E30133 for float's 24 bits, E30135 for double's 53 bits); representable,
// power-of-two, and boundary literals are not. A non-constant source cannot be
// proven lossy at compile time and is pervasive in shader code, so it is
// diagnosed only under -Wpedantic (E30134 for float, E30136 for double), and
// only when the source type is wide enough to actually lose precision. Folded
// constant *expressions* are intentionally not diagnosed (constant folding does
// not wrap at each typed operation, so a wrapping expression's folded value
// would false-positive); nor are vector/matrix targets (scalar targets only).
//
// The matcher is exhaustive over the scalar conversions below: every emitted
// diagnostic must have a matching annotation, so a source line with no
// annotation also asserts it produces no diagnostic. Each warning emits two rows
// -- a summary and a detailed span message -- matched by substring against the
// emitted diagnostics independent of source order, so the annotations are
// grouped (all summaries, then all spans) and repeat once per source line that
// produces the same text. At least one `warning E<code>` annotation per new
// diagnostic pins the severity+code, asserting these are warnings and not errors
// -- the central design choice: a lossy literal must not fail compilation.
// See #12929.

// Default: only the provably-lossy literal conversions warn.
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT):
//DEFAULT: warning E30133
//DEFAULT: integer-to-float conversion loses precision
//DEFAULT: integer-to-float conversion loses precision
//DEFAULT: integer-to-float conversion loses precision
//DEFAULT: integer-to-float conversion loses precision
//DEFAULT: integer-to-float conversion loses precision
//DEFAULT: 'int' to 'float' loses precision
//DEFAULT: 'int' to 'float' loses precision
//DEFAULT: 'int' to 'float' loses precision
//DEFAULT: 'int' to 'float' loses precision
//DEFAULT: 'uint' to 'float' loses precision
//DEFAULT: 'int64_t' to 'float' loses precision
//DEFAULT: warning E30135
//DEFAULT: integer-to-double conversion loses precision
//DEFAULT: 'int64_t' to 'double' loses precision
//DEFAULT: 'uint64_t' to 'double' loses precision

// -Wpedantic additionally flags non-constant integer sources wide enough to lose precision.
//DIAGNOSTIC_TEST:SIMPLE(diag=PEDANTIC): -Wpedantic
//PEDANTIC: warning E30133
//PEDANTIC: integer-to-float conversion loses precision
//PEDANTIC: integer-to-float conversion loses precision
//PEDANTIC: integer-to-float conversion loses precision
//PEDANTIC: integer-to-float conversion loses precision
//PEDANTIC: integer-to-float conversion loses precision
//PEDANTIC: 'int' to 'float' loses precision
//PEDANTIC: 'int' to 'float' loses precision
//PEDANTIC: 'int' to 'float' loses precision
//PEDANTIC: 'int' to 'float' loses precision
//PEDANTIC: 'uint' to 'float' loses precision
//PEDANTIC: 'int64_t' to 'float' loses precision
//PEDANTIC: warning E30134
//PEDANTIC: 'int' to 'float' may lose precision
//PEDANTIC: warning E30135
//PEDANTIC: integer-to-double conversion loses precision
//PEDANTIC: 'int64_t' to 'double' loses precision
//PEDANTIC: 'uint64_t' to 'double' loses precision
//PEDANTIC: warning E30136
//PEDANTIC: 'int64_t' to 'double' may lose precision

void testConversions(int i, int64_t l)
{
    // Literal not representable in float's significand -> warns by default.
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

    // 2^24 + 1: the smallest integer above that run that is NOT representable
    // (25 significant bits, odd) -> warns. Pins the float 24->25-bit boundary
    // with an odd value; the power-of-two boundaries above strip to a single
    // significant bit and never exercise the `significantBits <= 24` compare.
    float boundPlus = 16777217;

    // 2^24 - 1: all 24 bits set, still representable -> never warns.
    float boundExact = 16777215;

    // uint source (not int): 2^24 + 1 is not representable -> warns. Exercises
    // the 'uint' to 'float' span and the unsigned 32-bit magnitude path.
    float u32 = 16777217u;

    // int64_t source -> float: 2^53 + 1 needs 54 significant bits, not
    // representable in float's 24 -> warns. Exercises the 'int64_t' to 'float'
    // span (float target with a 64-bit source).
    float i64f = 9007199254740993;

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

    // Vector target: not a BasicExpressionType, so the check is skipped (scalar
    // targets only). A lossy scalar-literal broadcast is silently NOT diagnosed
    // -> never warns. Locks the documented scalar-only scope (see #12929).
    float3 vbroadcast = 123456789;

    // Non-constant int -> float: only under -Wpedantic.
    float e = i;

    // Literal not representable in double's significand (2^53 + 1) -> warns by default.
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
