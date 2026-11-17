//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

// Front-end constant folding of the integer bitwise operators `~ & | ^` and the shifts
// `<< >>` on int8_t, uint8_t, int64_t and uint64_t operands. Shift counts have the same
// type as the shifted value.
//
// Each case is a `static const int` that is 1 when the folded expression has the expected
// value and 0 otherwise. A `switch` then uses the constant as a second case label next to
// `case 0`, which makes the front end fold it to a concrete value:
//
// - a wrong result folds to 0 and fails with "duplicate cases in switch" at the `case` line,
// - a result that does not fold to a concrete value fails with "expression does not evaluate to
//   a compile-time constant" or "could not extract value from integer constant".
//
// An array size such as `int x[cond ? 1 : -1]` is not used, because an operator that fails to
// fold on the built-in path leaves a symbolic array size behind without any diagnostic.
//
// The `_b` cases spell the operator infix or prefix on builtin scalars, which the checker
// rewrites to a `BuiltinOperatorExpr` (`convertToBuiltinArithmeticOp`) folded by
// `tryConstantFoldBuiltinOperatorExpr`. The `_n` cases call `operator OP(...)` explicitly, which
// stays an `InvokeExpr` folded by the operator table in `SemanticsVisitor::tryConstantFoldExpr`.
// Both paths end in `BuiltinOperationIntVal::tryFoldImpl`.
//
// Expected values follow two's-complement wrapping at the operand width, division truncating
// toward zero, and arithmetic right shift for signed types. None of the cases may produce a
// diagnostic: the test runs in exhaustive mode with no annotations.

// ------------------------------------------------------------------------------------------------
// bitnot `~`
// ------------------------------------------------------------------------------------------------

static const int bitnot_i8_zero_b = ((~int8_t(0)) == int8_t(-1)) ? 1 : 0;
static const int bitnot_i8_zero_n = ((operator~(int8_t(0))) == int8_t(-1)) ? 1 : 0;
static const int bitnot_i8_min_b = ((~int8_t(-128)) == int8_t(127)) ? 1 : 0;
static const int bitnot_i8_min_n = ((operator~(int8_t(-128))) == int8_t(127)) ? 1 : 0;
static const int bitnot_i8_max_b = ((~int8_t(127)) == int8_t(-128)) ? 1 : 0;
static const int bitnot_i8_max_n = ((operator~(int8_t(127))) == int8_t(-128)) ? 1 : 0;
static const int bitnot_i8_low_half_b = ((~int8_t(15)) == int8_t(-16)) ? 1 : 0;
static const int bitnot_i8_low_half_n = ((operator~(int8_t(15))) == int8_t(-16)) ? 1 : 0;
static const int bitnot_i8_alternating_b = ((~int8_t(-91)) == int8_t(90)) ? 1 : 0;
static const int bitnot_i8_alternating_n = ((operator~(int8_t(-91))) == int8_t(90)) ? 1 : 0;
static const int bitnot_u8_zero_b = ((~uint8_t(0x00)) == uint8_t(0xFF)) ? 1 : 0;
static const int bitnot_u8_zero_n = ((operator~(uint8_t(0x00))) == uint8_t(0xFF)) ? 1 : 0;
static const int bitnot_u8_max_b = ((~uint8_t(0xFF)) == uint8_t(0x00)) ? 1 : 0;
static const int bitnot_u8_max_n = ((operator~(uint8_t(0xFF))) == uint8_t(0x00)) ? 1 : 0;
static const int bitnot_u8_low_half_b = ((~uint8_t(0x0F)) == uint8_t(0xF0)) ? 1 : 0;
static const int bitnot_u8_low_half_n = ((operator~(uint8_t(0x0F))) == uint8_t(0xF0)) ? 1 : 0;
static const int bitnot_u8_alternating_b = ((~uint8_t(0xA5)) == uint8_t(0x5A)) ? 1 : 0;
static const int bitnot_u8_alternating_n = ((operator~(uint8_t(0xA5))) == uint8_t(0x5A)) ? 1 : 0;
static const int bitnot_i64_zero_b = ((~int64_t(0)) == int64_t(-1)) ? 1 : 0;
static const int bitnot_i64_zero_n = ((operator~(int64_t(0))) == int64_t(-1)) ? 1 : 0;
static const int bitnot_i64_min_b = ((~int64_t(0x8000000000000000ull)) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitnot_i64_min_n = ((operator~(int64_t(0x8000000000000000ull))) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitnot_i64_max_b = ((~int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int bitnot_i64_max_n = ((operator~(int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int bitnot_i64_low_half_b = ((~int64_t(0x00000000FFFFFFFFull)) == int64_t(0xFFFFFFFF00000000ull)) ? 1 : 0;
static const int bitnot_i64_low_half_n = ((operator~(int64_t(0x00000000FFFFFFFFull))) == int64_t(0xFFFFFFFF00000000ull)) ? 1 : 0;
static const int bitnot_i64_alternating_b = ((~int64_t(0xA5A5A5A5A5A5A5A5ull)) == int64_t(0x5A5A5A5A5A5A5A5Aull)) ? 1 : 0;
static const int bitnot_i64_alternating_n = ((operator~(int64_t(0xA5A5A5A5A5A5A5A5ull))) == int64_t(0x5A5A5A5A5A5A5A5Aull)) ? 1 : 0;
static const int bitnot_u64_zero_b = ((~uint64_t(0)) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitnot_u64_zero_n = ((operator~(uint64_t(0))) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitnot_u64_max_b = ((~uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0)) ? 1 : 0;
static const int bitnot_u64_max_n = ((operator~(uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(0)) ? 1 : 0;
static const int bitnot_u64_low_half_b = ((~uint64_t(0x00000000FFFFFFFFull)) == uint64_t(0xFFFFFFFF00000000ull)) ? 1 : 0;
static const int bitnot_u64_low_half_n = ((operator~(uint64_t(0x00000000FFFFFFFFull))) == uint64_t(0xFFFFFFFF00000000ull)) ? 1 : 0;
static const int bitnot_u64_alternating_b = ((~uint64_t(0xA5A5A5A5A5A5A5A5ull)) == uint64_t(0x5A5A5A5A5A5A5A5Aull)) ? 1 : 0;
static const int bitnot_u64_alternating_n = ((operator~(uint64_t(0xA5A5A5A5A5A5A5A5ull))) == uint64_t(0x5A5A5A5A5A5A5A5Aull)) ? 1 : 0;

void check_bitnot()
{
    switch (0) { case 0: case bitnot_i8_zero_b: break; }
    switch (0) { case 0: case bitnot_i8_zero_n: break; }
    switch (0) { case 0: case bitnot_i8_min_b: break; }
    switch (0) { case 0: case bitnot_i8_min_n: break; }
    switch (0) { case 0: case bitnot_i8_max_b: break; }
    switch (0) { case 0: case bitnot_i8_max_n: break; }
    switch (0) { case 0: case bitnot_i8_low_half_b: break; }
    switch (0) { case 0: case bitnot_i8_low_half_n: break; }
    switch (0) { case 0: case bitnot_i8_alternating_b: break; }
    switch (0) { case 0: case bitnot_i8_alternating_n: break; }
    switch (0) { case 0: case bitnot_u8_zero_b: break; }
    switch (0) { case 0: case bitnot_u8_zero_n: break; }
    switch (0) { case 0: case bitnot_u8_max_b: break; }
    switch (0) { case 0: case bitnot_u8_max_n: break; }
    switch (0) { case 0: case bitnot_u8_low_half_b: break; }
    switch (0) { case 0: case bitnot_u8_low_half_n: break; }
    switch (0) { case 0: case bitnot_u8_alternating_b: break; }
    switch (0) { case 0: case bitnot_u8_alternating_n: break; }
    switch (0) { case 0: case bitnot_i64_zero_b: break; }
    switch (0) { case 0: case bitnot_i64_zero_n: break; }
    switch (0) { case 0: case bitnot_i64_min_b: break; }
    switch (0) { case 0: case bitnot_i64_min_n: break; }
    switch (0) { case 0: case bitnot_i64_max_b: break; }
    switch (0) { case 0: case bitnot_i64_max_n: break; }
    switch (0) { case 0: case bitnot_i64_low_half_b: break; }
    switch (0) { case 0: case bitnot_i64_low_half_n: break; }
    switch (0) { case 0: case bitnot_i64_alternating_b: break; }
    switch (0) { case 0: case bitnot_i64_alternating_n: break; }
    switch (0) { case 0: case bitnot_u64_zero_b: break; }
    switch (0) { case 0: case bitnot_u64_zero_n: break; }
    switch (0) { case 0: case bitnot_u64_max_b: break; }
    switch (0) { case 0: case bitnot_u64_max_n: break; }
    switch (0) { case 0: case bitnot_u64_low_half_b: break; }
    switch (0) { case 0: case bitnot_u64_low_half_n: break; }
    switch (0) { case 0: case bitnot_u64_alternating_b: break; }
    switch (0) { case 0: case bitnot_u64_alternating_n: break; }
}

// ------------------------------------------------------------------------------------------------
// bitand `&`
// ------------------------------------------------------------------------------------------------

static const int bitand_i8_high_half_low_half_b = ((int8_t(-16) & int8_t(15)) == int8_t(0)) ? 1 : 0;
static const int bitand_i8_high_half_low_half_n = ((operator&(int8_t(-16), int8_t(15))) == int8_t(0)) ? 1 : 0;
static const int bitand_i8_all_ones_alternating_b = ((int8_t(-1) & int8_t(-91)) == int8_t(-91)) ? 1 : 0;
static const int bitand_i8_all_ones_alternating_n = ((operator&(int8_t(-1), int8_t(-91))) == int8_t(-91)) ? 1 : 0;
static const int bitand_i8_top_bit_max_b = ((int8_t(-128) & int8_t(127)) == int8_t(0)) ? 1 : 0;
static const int bitand_i8_top_bit_max_n = ((operator&(int8_t(-128), int8_t(127))) == int8_t(0)) ? 1 : 0;
static const int bitand_i8_zero_alternating_b = ((int8_t(0) & int8_t(-91)) == int8_t(0)) ? 1 : 0;
static const int bitand_i8_zero_alternating_n = ((operator&(int8_t(0), int8_t(-91))) == int8_t(0)) ? 1 : 0;
static const int bitand_i8_alternating_low_half_b = ((int8_t(-91) & int8_t(15)) == int8_t(5)) ? 1 : 0;
static const int bitand_i8_alternating_low_half_n = ((operator&(int8_t(-91), int8_t(15))) == int8_t(5)) ? 1 : 0;
static const int bitand_u8_high_half_low_half_b = ((uint8_t(0xF0) & uint8_t(0x0F)) == uint8_t(0x00)) ? 1 : 0;
static const int bitand_u8_high_half_low_half_n = ((operator&(uint8_t(0xF0), uint8_t(0x0F))) == uint8_t(0x00)) ? 1 : 0;
static const int bitand_u8_all_ones_alternating_b = ((uint8_t(0xFF) & uint8_t(0xA5)) == uint8_t(0xA5)) ? 1 : 0;
static const int bitand_u8_all_ones_alternating_n = ((operator&(uint8_t(0xFF), uint8_t(0xA5))) == uint8_t(0xA5)) ? 1 : 0;
static const int bitand_u8_top_bit_max_b = ((uint8_t(0x80) & uint8_t(0xFF)) == uint8_t(0x80)) ? 1 : 0;
static const int bitand_u8_top_bit_max_n = ((operator&(uint8_t(0x80), uint8_t(0xFF))) == uint8_t(0x80)) ? 1 : 0;
static const int bitand_u8_zero_alternating_b = ((uint8_t(0x00) & uint8_t(0xA5)) == uint8_t(0x00)) ? 1 : 0;
static const int bitand_u8_zero_alternating_n = ((operator&(uint8_t(0x00), uint8_t(0xA5))) == uint8_t(0x00)) ? 1 : 0;
static const int bitand_u8_alternating_low_half_b = ((uint8_t(0xA5) & uint8_t(0x0F)) == uint8_t(0x05)) ? 1 : 0;
static const int bitand_u8_alternating_low_half_n = ((operator&(uint8_t(0xA5), uint8_t(0x0F))) == uint8_t(0x05)) ? 1 : 0;
static const int bitand_i64_high_half_low_half_b = ((int64_t(0xFFFFFFFF00000000ull) & int64_t(0x00000000FFFFFFFFull)) == int64_t(0)) ? 1 : 0;
static const int bitand_i64_high_half_low_half_n = ((operator&(int64_t(0xFFFFFFFF00000000ull), int64_t(0x00000000FFFFFFFFull))) == int64_t(0)) ? 1 : 0;
static const int bitand_i64_all_ones_alternating_b = ((int64_t(-1) & int64_t(0xA5A5A5A5A5A5A5A5ull)) == int64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitand_i64_all_ones_alternating_n = ((operator&(int64_t(-1), int64_t(0xA5A5A5A5A5A5A5A5ull))) == int64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitand_i64_top_bit_max_b = ((int64_t(0x8000000000000000ull) & int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(0)) ? 1 : 0;
static const int bitand_i64_top_bit_max_n = ((operator&(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(0)) ? 1 : 0;
static const int bitand_i64_zero_alternating_b = ((int64_t(0) & int64_t(0xA5A5A5A5A5A5A5A5ull)) == int64_t(0)) ? 1 : 0;
static const int bitand_i64_zero_alternating_n = ((operator&(int64_t(0), int64_t(0xA5A5A5A5A5A5A5A5ull))) == int64_t(0)) ? 1 : 0;
static const int bitand_i64_alternating_low_half_b = ((int64_t(0xA5A5A5A5A5A5A5A5ull) & int64_t(0x00000000FFFFFFFFull)) == int64_t(0x00000000A5A5A5A5ull)) ? 1 : 0;
static const int bitand_i64_alternating_low_half_n = ((operator&(int64_t(0xA5A5A5A5A5A5A5A5ull), int64_t(0x00000000FFFFFFFFull))) == int64_t(0x00000000A5A5A5A5ull)) ? 1 : 0;
static const int bitand_u64_high_half_low_half_b = ((uint64_t(0xFFFFFFFF00000000ull) & uint64_t(0x00000000FFFFFFFFull)) == uint64_t(0)) ? 1 : 0;
static const int bitand_u64_high_half_low_half_n = ((operator&(uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x00000000FFFFFFFFull))) == uint64_t(0)) ? 1 : 0;
static const int bitand_u64_all_ones_alternating_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) & uint64_t(0xA5A5A5A5A5A5A5A5ull)) == uint64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitand_u64_all_ones_alternating_n = ((operator&(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0xA5A5A5A5A5A5A5A5ull))) == uint64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitand_u64_top_bit_max_b = ((uint64_t(0x8000000000000000ull) & uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0x8000000000000000ull)) ? 1 : 0;
static const int bitand_u64_top_bit_max_n = ((operator&(uint64_t(0x8000000000000000ull), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(0x8000000000000000ull)) ? 1 : 0;
static const int bitand_u64_zero_alternating_b = ((uint64_t(0) & uint64_t(0xA5A5A5A5A5A5A5A5ull)) == uint64_t(0)) ? 1 : 0;
static const int bitand_u64_zero_alternating_n = ((operator&(uint64_t(0), uint64_t(0xA5A5A5A5A5A5A5A5ull))) == uint64_t(0)) ? 1 : 0;
static const int bitand_u64_alternating_low_half_b = ((uint64_t(0xA5A5A5A5A5A5A5A5ull) & uint64_t(0x00000000FFFFFFFFull)) == uint64_t(0x00000000A5A5A5A5ull)) ? 1 : 0;
static const int bitand_u64_alternating_low_half_n = ((operator&(uint64_t(0xA5A5A5A5A5A5A5A5ull), uint64_t(0x00000000FFFFFFFFull))) == uint64_t(0x00000000A5A5A5A5ull)) ? 1 : 0;

void check_bitand()
{
    switch (0) { case 0: case bitand_i8_high_half_low_half_b: break; }
    switch (0) { case 0: case bitand_i8_high_half_low_half_n: break; }
    switch (0) { case 0: case bitand_i8_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitand_i8_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitand_i8_top_bit_max_b: break; }
    switch (0) { case 0: case bitand_i8_top_bit_max_n: break; }
    switch (0) { case 0: case bitand_i8_zero_alternating_b: break; }
    switch (0) { case 0: case bitand_i8_zero_alternating_n: break; }
    switch (0) { case 0: case bitand_i8_alternating_low_half_b: break; }
    switch (0) { case 0: case bitand_i8_alternating_low_half_n: break; }
    switch (0) { case 0: case bitand_u8_high_half_low_half_b: break; }
    switch (0) { case 0: case bitand_u8_high_half_low_half_n: break; }
    switch (0) { case 0: case bitand_u8_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitand_u8_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitand_u8_top_bit_max_b: break; }
    switch (0) { case 0: case bitand_u8_top_bit_max_n: break; }
    switch (0) { case 0: case bitand_u8_zero_alternating_b: break; }
    switch (0) { case 0: case bitand_u8_zero_alternating_n: break; }
    switch (0) { case 0: case bitand_u8_alternating_low_half_b: break; }
    switch (0) { case 0: case bitand_u8_alternating_low_half_n: break; }
    switch (0) { case 0: case bitand_i64_high_half_low_half_b: break; }
    switch (0) { case 0: case bitand_i64_high_half_low_half_n: break; }
    switch (0) { case 0: case bitand_i64_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitand_i64_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitand_i64_top_bit_max_b: break; }
    switch (0) { case 0: case bitand_i64_top_bit_max_n: break; }
    switch (0) { case 0: case bitand_i64_zero_alternating_b: break; }
    switch (0) { case 0: case bitand_i64_zero_alternating_n: break; }
    switch (0) { case 0: case bitand_i64_alternating_low_half_b: break; }
    switch (0) { case 0: case bitand_i64_alternating_low_half_n: break; }
    switch (0) { case 0: case bitand_u64_high_half_low_half_b: break; }
    switch (0) { case 0: case bitand_u64_high_half_low_half_n: break; }
    switch (0) { case 0: case bitand_u64_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitand_u64_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitand_u64_top_bit_max_b: break; }
    switch (0) { case 0: case bitand_u64_top_bit_max_n: break; }
    switch (0) { case 0: case bitand_u64_zero_alternating_b: break; }
    switch (0) { case 0: case bitand_u64_zero_alternating_n: break; }
    switch (0) { case 0: case bitand_u64_alternating_low_half_b: break; }
    switch (0) { case 0: case bitand_u64_alternating_low_half_n: break; }
}

// ------------------------------------------------------------------------------------------------
// bitor `|`
// ------------------------------------------------------------------------------------------------

static const int bitor_i8_high_half_low_half_b = ((int8_t(-16) | int8_t(15)) == int8_t(-1)) ? 1 : 0;
static const int bitor_i8_high_half_low_half_n = ((operator|(int8_t(-16), int8_t(15))) == int8_t(-1)) ? 1 : 0;
static const int bitor_i8_all_ones_alternating_b = ((int8_t(-1) | int8_t(-91)) == int8_t(-1)) ? 1 : 0;
static const int bitor_i8_all_ones_alternating_n = ((operator|(int8_t(-1), int8_t(-91))) == int8_t(-1)) ? 1 : 0;
static const int bitor_i8_top_bit_max_b = ((int8_t(-128) | int8_t(127)) == int8_t(-1)) ? 1 : 0;
static const int bitor_i8_top_bit_max_n = ((operator|(int8_t(-128), int8_t(127))) == int8_t(-1)) ? 1 : 0;
static const int bitor_i8_zero_alternating_b = ((int8_t(0) | int8_t(-91)) == int8_t(-91)) ? 1 : 0;
static const int bitor_i8_zero_alternating_n = ((operator|(int8_t(0), int8_t(-91))) == int8_t(-91)) ? 1 : 0;
static const int bitor_i8_alternating_low_half_b = ((int8_t(-91) | int8_t(15)) == int8_t(-81)) ? 1 : 0;
static const int bitor_i8_alternating_low_half_n = ((operator|(int8_t(-91), int8_t(15))) == int8_t(-81)) ? 1 : 0;
static const int bitor_u8_high_half_low_half_b = ((uint8_t(0xF0) | uint8_t(0x0F)) == uint8_t(0xFF)) ? 1 : 0;
static const int bitor_u8_high_half_low_half_n = ((operator|(uint8_t(0xF0), uint8_t(0x0F))) == uint8_t(0xFF)) ? 1 : 0;
static const int bitor_u8_all_ones_alternating_b = ((uint8_t(0xFF) | uint8_t(0xA5)) == uint8_t(0xFF)) ? 1 : 0;
static const int bitor_u8_all_ones_alternating_n = ((operator|(uint8_t(0xFF), uint8_t(0xA5))) == uint8_t(0xFF)) ? 1 : 0;
static const int bitor_u8_top_bit_max_b = ((uint8_t(0x80) | uint8_t(0xFF)) == uint8_t(0xFF)) ? 1 : 0;
static const int bitor_u8_top_bit_max_n = ((operator|(uint8_t(0x80), uint8_t(0xFF))) == uint8_t(0xFF)) ? 1 : 0;
static const int bitor_u8_zero_alternating_b = ((uint8_t(0x00) | uint8_t(0xA5)) == uint8_t(0xA5)) ? 1 : 0;
static const int bitor_u8_zero_alternating_n = ((operator|(uint8_t(0x00), uint8_t(0xA5))) == uint8_t(0xA5)) ? 1 : 0;
static const int bitor_u8_alternating_low_half_b = ((uint8_t(0xA5) | uint8_t(0x0F)) == uint8_t(0xAF)) ? 1 : 0;
static const int bitor_u8_alternating_low_half_n = ((operator|(uint8_t(0xA5), uint8_t(0x0F))) == uint8_t(0xAF)) ? 1 : 0;
static const int bitor_i64_high_half_low_half_b = ((int64_t(0xFFFFFFFF00000000ull) | int64_t(0x00000000FFFFFFFFull)) == int64_t(-1)) ? 1 : 0;
static const int bitor_i64_high_half_low_half_n = ((operator|(int64_t(0xFFFFFFFF00000000ull), int64_t(0x00000000FFFFFFFFull))) == int64_t(-1)) ? 1 : 0;
static const int bitor_i64_all_ones_alternating_b = ((int64_t(-1) | int64_t(0xA5A5A5A5A5A5A5A5ull)) == int64_t(-1)) ? 1 : 0;
static const int bitor_i64_all_ones_alternating_n = ((operator|(int64_t(-1), int64_t(0xA5A5A5A5A5A5A5A5ull))) == int64_t(-1)) ? 1 : 0;
static const int bitor_i64_top_bit_max_b = ((int64_t(0x8000000000000000ull) | int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(-1)) ? 1 : 0;
static const int bitor_i64_top_bit_max_n = ((operator|(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(-1)) ? 1 : 0;
static const int bitor_i64_zero_alternating_b = ((int64_t(0) | int64_t(0xA5A5A5A5A5A5A5A5ull)) == int64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitor_i64_zero_alternating_n = ((operator|(int64_t(0), int64_t(0xA5A5A5A5A5A5A5A5ull))) == int64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitor_i64_alternating_low_half_b = ((int64_t(0xA5A5A5A5A5A5A5A5ull) | int64_t(0x00000000FFFFFFFFull)) == int64_t(0xA5A5A5A5FFFFFFFFull)) ? 1 : 0;
static const int bitor_i64_alternating_low_half_n = ((operator|(int64_t(0xA5A5A5A5A5A5A5A5ull), int64_t(0x00000000FFFFFFFFull))) == int64_t(0xA5A5A5A5FFFFFFFFull)) ? 1 : 0;
static const int bitor_u64_high_half_low_half_b = ((uint64_t(0xFFFFFFFF00000000ull) | uint64_t(0x00000000FFFFFFFFull)) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitor_u64_high_half_low_half_n = ((operator|(uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x00000000FFFFFFFFull))) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitor_u64_all_ones_alternating_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) | uint64_t(0xA5A5A5A5A5A5A5A5ull)) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitor_u64_all_ones_alternating_n = ((operator|(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0xA5A5A5A5A5A5A5A5ull))) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitor_u64_top_bit_max_b = ((uint64_t(0x8000000000000000ull) | uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitor_u64_top_bit_max_n = ((operator|(uint64_t(0x8000000000000000ull), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitor_u64_zero_alternating_b = ((uint64_t(0) | uint64_t(0xA5A5A5A5A5A5A5A5ull)) == uint64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitor_u64_zero_alternating_n = ((operator|(uint64_t(0), uint64_t(0xA5A5A5A5A5A5A5A5ull))) == uint64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitor_u64_alternating_low_half_b = ((uint64_t(0xA5A5A5A5A5A5A5A5ull) | uint64_t(0x00000000FFFFFFFFull)) == uint64_t(0xA5A5A5A5FFFFFFFFull)) ? 1 : 0;
static const int bitor_u64_alternating_low_half_n = ((operator|(uint64_t(0xA5A5A5A5A5A5A5A5ull), uint64_t(0x00000000FFFFFFFFull))) == uint64_t(0xA5A5A5A5FFFFFFFFull)) ? 1 : 0;

void check_bitor()
{
    switch (0) { case 0: case bitor_i8_high_half_low_half_b: break; }
    switch (0) { case 0: case bitor_i8_high_half_low_half_n: break; }
    switch (0) { case 0: case bitor_i8_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitor_i8_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitor_i8_top_bit_max_b: break; }
    switch (0) { case 0: case bitor_i8_top_bit_max_n: break; }
    switch (0) { case 0: case bitor_i8_zero_alternating_b: break; }
    switch (0) { case 0: case bitor_i8_zero_alternating_n: break; }
    switch (0) { case 0: case bitor_i8_alternating_low_half_b: break; }
    switch (0) { case 0: case bitor_i8_alternating_low_half_n: break; }
    switch (0) { case 0: case bitor_u8_high_half_low_half_b: break; }
    switch (0) { case 0: case bitor_u8_high_half_low_half_n: break; }
    switch (0) { case 0: case bitor_u8_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitor_u8_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitor_u8_top_bit_max_b: break; }
    switch (0) { case 0: case bitor_u8_top_bit_max_n: break; }
    switch (0) { case 0: case bitor_u8_zero_alternating_b: break; }
    switch (0) { case 0: case bitor_u8_zero_alternating_n: break; }
    switch (0) { case 0: case bitor_u8_alternating_low_half_b: break; }
    switch (0) { case 0: case bitor_u8_alternating_low_half_n: break; }
    switch (0) { case 0: case bitor_i64_high_half_low_half_b: break; }
    switch (0) { case 0: case bitor_i64_high_half_low_half_n: break; }
    switch (0) { case 0: case bitor_i64_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitor_i64_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitor_i64_top_bit_max_b: break; }
    switch (0) { case 0: case bitor_i64_top_bit_max_n: break; }
    switch (0) { case 0: case bitor_i64_zero_alternating_b: break; }
    switch (0) { case 0: case bitor_i64_zero_alternating_n: break; }
    switch (0) { case 0: case bitor_i64_alternating_low_half_b: break; }
    switch (0) { case 0: case bitor_i64_alternating_low_half_n: break; }
    switch (0) { case 0: case bitor_u64_high_half_low_half_b: break; }
    switch (0) { case 0: case bitor_u64_high_half_low_half_n: break; }
    switch (0) { case 0: case bitor_u64_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitor_u64_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitor_u64_top_bit_max_b: break; }
    switch (0) { case 0: case bitor_u64_top_bit_max_n: break; }
    switch (0) { case 0: case bitor_u64_zero_alternating_b: break; }
    switch (0) { case 0: case bitor_u64_zero_alternating_n: break; }
    switch (0) { case 0: case bitor_u64_alternating_low_half_b: break; }
    switch (0) { case 0: case bitor_u64_alternating_low_half_n: break; }
}

// ------------------------------------------------------------------------------------------------
// bitxor `^`
// ------------------------------------------------------------------------------------------------

static const int bitxor_i8_high_half_low_half_b = ((int8_t(-16) ^ int8_t(15)) == int8_t(-1)) ? 1 : 0;
static const int bitxor_i8_high_half_low_half_n = ((operator^(int8_t(-16), int8_t(15))) == int8_t(-1)) ? 1 : 0;
static const int bitxor_i8_all_ones_alternating_b = ((int8_t(-1) ^ int8_t(-91)) == int8_t(90)) ? 1 : 0;
static const int bitxor_i8_all_ones_alternating_n = ((operator^(int8_t(-1), int8_t(-91))) == int8_t(90)) ? 1 : 0;
static const int bitxor_i8_top_bit_max_b = ((int8_t(-128) ^ int8_t(127)) == int8_t(-1)) ? 1 : 0;
static const int bitxor_i8_top_bit_max_n = ((operator^(int8_t(-128), int8_t(127))) == int8_t(-1)) ? 1 : 0;
static const int bitxor_i8_zero_alternating_b = ((int8_t(0) ^ int8_t(-91)) == int8_t(-91)) ? 1 : 0;
static const int bitxor_i8_zero_alternating_n = ((operator^(int8_t(0), int8_t(-91))) == int8_t(-91)) ? 1 : 0;
static const int bitxor_i8_alternating_low_half_b = ((int8_t(-91) ^ int8_t(15)) == int8_t(-86)) ? 1 : 0;
static const int bitxor_i8_alternating_low_half_n = ((operator^(int8_t(-91), int8_t(15))) == int8_t(-86)) ? 1 : 0;
static const int bitxor_u8_high_half_low_half_b = ((uint8_t(0xF0) ^ uint8_t(0x0F)) == uint8_t(0xFF)) ? 1 : 0;
static const int bitxor_u8_high_half_low_half_n = ((operator^(uint8_t(0xF0), uint8_t(0x0F))) == uint8_t(0xFF)) ? 1 : 0;
static const int bitxor_u8_all_ones_alternating_b = ((uint8_t(0xFF) ^ uint8_t(0xA5)) == uint8_t(0x5A)) ? 1 : 0;
static const int bitxor_u8_all_ones_alternating_n = ((operator^(uint8_t(0xFF), uint8_t(0xA5))) == uint8_t(0x5A)) ? 1 : 0;
static const int bitxor_u8_top_bit_max_b = ((uint8_t(0x80) ^ uint8_t(0xFF)) == uint8_t(0x7F)) ? 1 : 0;
static const int bitxor_u8_top_bit_max_n = ((operator^(uint8_t(0x80), uint8_t(0xFF))) == uint8_t(0x7F)) ? 1 : 0;
static const int bitxor_u8_zero_alternating_b = ((uint8_t(0x00) ^ uint8_t(0xA5)) == uint8_t(0xA5)) ? 1 : 0;
static const int bitxor_u8_zero_alternating_n = ((operator^(uint8_t(0x00), uint8_t(0xA5))) == uint8_t(0xA5)) ? 1 : 0;
static const int bitxor_u8_alternating_low_half_b = ((uint8_t(0xA5) ^ uint8_t(0x0F)) == uint8_t(0xAA)) ? 1 : 0;
static const int bitxor_u8_alternating_low_half_n = ((operator^(uint8_t(0xA5), uint8_t(0x0F))) == uint8_t(0xAA)) ? 1 : 0;
static const int bitxor_i64_high_half_low_half_b = ((int64_t(0xFFFFFFFF00000000ull) ^ int64_t(0x00000000FFFFFFFFull)) == int64_t(-1)) ? 1 : 0;
static const int bitxor_i64_high_half_low_half_n = ((operator^(int64_t(0xFFFFFFFF00000000ull), int64_t(0x00000000FFFFFFFFull))) == int64_t(-1)) ? 1 : 0;
static const int bitxor_i64_all_ones_alternating_b = ((int64_t(-1) ^ int64_t(0xA5A5A5A5A5A5A5A5ull)) == int64_t(0x5A5A5A5A5A5A5A5Aull)) ? 1 : 0;
static const int bitxor_i64_all_ones_alternating_n = ((operator^(int64_t(-1), int64_t(0xA5A5A5A5A5A5A5A5ull))) == int64_t(0x5A5A5A5A5A5A5A5Aull)) ? 1 : 0;
static const int bitxor_i64_top_bit_max_b = ((int64_t(0x8000000000000000ull) ^ int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(-1)) ? 1 : 0;
static const int bitxor_i64_top_bit_max_n = ((operator^(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(-1)) ? 1 : 0;
static const int bitxor_i64_zero_alternating_b = ((int64_t(0) ^ int64_t(0xA5A5A5A5A5A5A5A5ull)) == int64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitxor_i64_zero_alternating_n = ((operator^(int64_t(0), int64_t(0xA5A5A5A5A5A5A5A5ull))) == int64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitxor_i64_alternating_low_half_b = ((int64_t(0xA5A5A5A5A5A5A5A5ull) ^ int64_t(0x00000000FFFFFFFFull)) == int64_t(0xA5A5A5A55A5A5A5Aull)) ? 1 : 0;
static const int bitxor_i64_alternating_low_half_n = ((operator^(int64_t(0xA5A5A5A5A5A5A5A5ull), int64_t(0x00000000FFFFFFFFull))) == int64_t(0xA5A5A5A55A5A5A5Aull)) ? 1 : 0;
static const int bitxor_u64_high_half_low_half_b = ((uint64_t(0xFFFFFFFF00000000ull) ^ uint64_t(0x00000000FFFFFFFFull)) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitxor_u64_high_half_low_half_n = ((operator^(uint64_t(0xFFFFFFFF00000000ull), uint64_t(0x00000000FFFFFFFFull))) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitxor_u64_all_ones_alternating_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) ^ uint64_t(0xA5A5A5A5A5A5A5A5ull)) == uint64_t(0x5A5A5A5A5A5A5A5Aull)) ? 1 : 0;
static const int bitxor_u64_all_ones_alternating_n = ((operator^(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0xA5A5A5A5A5A5A5A5ull))) == uint64_t(0x5A5A5A5A5A5A5A5Aull)) ? 1 : 0;
static const int bitxor_u64_top_bit_max_b = ((uint64_t(0x8000000000000000ull) ^ uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitxor_u64_top_bit_max_n = ((operator^(uint64_t(0x8000000000000000ull), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int bitxor_u64_zero_alternating_b = ((uint64_t(0) ^ uint64_t(0xA5A5A5A5A5A5A5A5ull)) == uint64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitxor_u64_zero_alternating_n = ((operator^(uint64_t(0), uint64_t(0xA5A5A5A5A5A5A5A5ull))) == uint64_t(0xA5A5A5A5A5A5A5A5ull)) ? 1 : 0;
static const int bitxor_u64_alternating_low_half_b = ((uint64_t(0xA5A5A5A5A5A5A5A5ull) ^ uint64_t(0x00000000FFFFFFFFull)) == uint64_t(0xA5A5A5A55A5A5A5Aull)) ? 1 : 0;
static const int bitxor_u64_alternating_low_half_n = ((operator^(uint64_t(0xA5A5A5A5A5A5A5A5ull), uint64_t(0x00000000FFFFFFFFull))) == uint64_t(0xA5A5A5A55A5A5A5Aull)) ? 1 : 0;

void check_bitxor()
{
    switch (0) { case 0: case bitxor_i8_high_half_low_half_b: break; }
    switch (0) { case 0: case bitxor_i8_high_half_low_half_n: break; }
    switch (0) { case 0: case bitxor_i8_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitxor_i8_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitxor_i8_top_bit_max_b: break; }
    switch (0) { case 0: case bitxor_i8_top_bit_max_n: break; }
    switch (0) { case 0: case bitxor_i8_zero_alternating_b: break; }
    switch (0) { case 0: case bitxor_i8_zero_alternating_n: break; }
    switch (0) { case 0: case bitxor_i8_alternating_low_half_b: break; }
    switch (0) { case 0: case bitxor_i8_alternating_low_half_n: break; }
    switch (0) { case 0: case bitxor_u8_high_half_low_half_b: break; }
    switch (0) { case 0: case bitxor_u8_high_half_low_half_n: break; }
    switch (0) { case 0: case bitxor_u8_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitxor_u8_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitxor_u8_top_bit_max_b: break; }
    switch (0) { case 0: case bitxor_u8_top_bit_max_n: break; }
    switch (0) { case 0: case bitxor_u8_zero_alternating_b: break; }
    switch (0) { case 0: case bitxor_u8_zero_alternating_n: break; }
    switch (0) { case 0: case bitxor_u8_alternating_low_half_b: break; }
    switch (0) { case 0: case bitxor_u8_alternating_low_half_n: break; }
    switch (0) { case 0: case bitxor_i64_high_half_low_half_b: break; }
    switch (0) { case 0: case bitxor_i64_high_half_low_half_n: break; }
    switch (0) { case 0: case bitxor_i64_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitxor_i64_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitxor_i64_top_bit_max_b: break; }
    switch (0) { case 0: case bitxor_i64_top_bit_max_n: break; }
    switch (0) { case 0: case bitxor_i64_zero_alternating_b: break; }
    switch (0) { case 0: case bitxor_i64_zero_alternating_n: break; }
    switch (0) { case 0: case bitxor_i64_alternating_low_half_b: break; }
    switch (0) { case 0: case bitxor_i64_alternating_low_half_n: break; }
    switch (0) { case 0: case bitxor_u64_high_half_low_half_b: break; }
    switch (0) { case 0: case bitxor_u64_high_half_low_half_n: break; }
    switch (0) { case 0: case bitxor_u64_all_ones_alternating_b: break; }
    switch (0) { case 0: case bitxor_u64_all_ones_alternating_n: break; }
    switch (0) { case 0: case bitxor_u64_top_bit_max_b: break; }
    switch (0) { case 0: case bitxor_u64_top_bit_max_n: break; }
    switch (0) { case 0: case bitxor_u64_zero_alternating_b: break; }
    switch (0) { case 0: case bitxor_u64_zero_alternating_n: break; }
    switch (0) { case 0: case bitxor_u64_alternating_low_half_b: break; }
    switch (0) { case 0: case bitxor_u64_alternating_low_half_n: break; }
}

// ------------------------------------------------------------------------------------------------
// lsh `<<`
// Shift counts at or above the operand width, and negative counts, are
// target-defined and live in the diagnostics tests.
// ------------------------------------------------------------------------------------------------

static const int lsh_i8_one_by_width_minus_one_b = ((int8_t(1) << int8_t(7)) == int8_t(-128)) ? 1 : 0;
static const int lsh_i8_one_by_width_minus_one_n = ((operator<<(int8_t(1), int8_t(7))) == int8_t(-128)) ? 1 : 0;
static const int lsh_i8_all_ones_by_four_b = ((int8_t(-1) << int8_t(4)) == int8_t(-16)) ? 1 : 0;
static const int lsh_i8_all_ones_by_four_n = ((operator<<(int8_t(-1), int8_t(4))) == int8_t(-16)) ? 1 : 0;
static const int lsh_i8_max_by_zero_b = ((int8_t(127) << int8_t(0)) == int8_t(127)) ? 1 : 0;
static const int lsh_i8_max_by_zero_n = ((operator<<(int8_t(127), int8_t(0))) == int8_t(127)) ? 1 : 0;
static const int lsh_i8_one_by_one_b = ((int8_t(1) << int8_t(1)) == int8_t(2)) ? 1 : 0;
static const int lsh_i8_one_by_one_n = ((operator<<(int8_t(1), int8_t(1))) == int8_t(2)) ? 1 : 0;
static const int lsh_i8_low_half_by_half_width_b = ((int8_t(15) << int8_t(4)) == int8_t(-16)) ? 1 : 0;
static const int lsh_i8_low_half_by_half_width_n = ((operator<<(int8_t(15), int8_t(4))) == int8_t(-16)) ? 1 : 0;
static const int lsh_i8_minus_one_by_one_b = ((int8_t(-1) << int8_t(1)) == int8_t(-2)) ? 1 : 0;
static const int lsh_i8_minus_one_by_one_n = ((operator<<(int8_t(-1), int8_t(1))) == int8_t(-2)) ? 1 : 0;
static const int lsh_i8_max_by_one_b = ((int8_t(127) << int8_t(1)) == int8_t(-2)) ? 1 : 0;
static const int lsh_i8_max_by_one_n = ((operator<<(int8_t(127), int8_t(1))) == int8_t(-2)) ? 1 : 0;
static const int lsh_u8_one_by_width_minus_one_b = ((uint8_t(0x01) << uint8_t(0x07)) == uint8_t(0x80)) ? 1 : 0;
static const int lsh_u8_one_by_width_minus_one_n = ((operator<<(uint8_t(0x01), uint8_t(0x07))) == uint8_t(0x80)) ? 1 : 0;
static const int lsh_u8_all_ones_by_four_b = ((uint8_t(0xFF) << uint8_t(0x04)) == uint8_t(0xF0)) ? 1 : 0;
static const int lsh_u8_all_ones_by_four_n = ((operator<<(uint8_t(0xFF), uint8_t(0x04))) == uint8_t(0xF0)) ? 1 : 0;
static const int lsh_u8_max_by_zero_b = ((uint8_t(0xFF) << uint8_t(0x00)) == uint8_t(0xFF)) ? 1 : 0;
static const int lsh_u8_max_by_zero_n = ((operator<<(uint8_t(0xFF), uint8_t(0x00))) == uint8_t(0xFF)) ? 1 : 0;
static const int lsh_u8_one_by_one_b = ((uint8_t(0x01) << uint8_t(0x01)) == uint8_t(0x02)) ? 1 : 0;
static const int lsh_u8_one_by_one_n = ((operator<<(uint8_t(0x01), uint8_t(0x01))) == uint8_t(0x02)) ? 1 : 0;
static const int lsh_u8_low_half_by_half_width_b = ((uint8_t(0x0F) << uint8_t(0x04)) == uint8_t(0xF0)) ? 1 : 0;
static const int lsh_u8_low_half_by_half_width_n = ((operator<<(uint8_t(0x0F), uint8_t(0x04))) == uint8_t(0xF0)) ? 1 : 0;
static const int lsh_i64_one_by_width_minus_one_b = ((int64_t(1) << int64_t(63)) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int lsh_i64_one_by_width_minus_one_n = ((operator<<(int64_t(1), int64_t(63))) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int lsh_i64_all_ones_by_four_b = ((int64_t(-1) << int64_t(4)) == int64_t(-16)) ? 1 : 0;
static const int lsh_i64_all_ones_by_four_n = ((operator<<(int64_t(-1), int64_t(4))) == int64_t(-16)) ? 1 : 0;
static const int lsh_i64_max_by_zero_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) << int64_t(0)) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int lsh_i64_max_by_zero_n = ((operator<<(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0))) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int lsh_i64_one_by_one_b = ((int64_t(1) << int64_t(1)) == int64_t(2)) ? 1 : 0;
static const int lsh_i64_one_by_one_n = ((operator<<(int64_t(1), int64_t(1))) == int64_t(2)) ? 1 : 0;
static const int lsh_i64_low_half_by_half_width_b = ((int64_t(0x00000000FFFFFFFFull) << int64_t(32)) == int64_t(0xFFFFFFFF00000000ull)) ? 1 : 0;
static const int lsh_i64_low_half_by_half_width_n = ((operator<<(int64_t(0x00000000FFFFFFFFull), int64_t(32))) == int64_t(0xFFFFFFFF00000000ull)) ? 1 : 0;
static const int lsh_i64_minus_one_by_one_b = ((int64_t(-1) << int64_t(1)) == int64_t(-2)) ? 1 : 0;
static const int lsh_i64_minus_one_by_one_n = ((operator<<(int64_t(-1), int64_t(1))) == int64_t(-2)) ? 1 : 0;
static const int lsh_i64_max_by_one_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) << int64_t(1)) == int64_t(-2)) ? 1 : 0;
static const int lsh_i64_max_by_one_n = ((operator<<(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(1))) == int64_t(-2)) ? 1 : 0;
static const int lsh_u64_one_by_width_minus_one_b = ((uint64_t(1) << uint64_t(63)) == uint64_t(0x8000000000000000ull)) ? 1 : 0;
static const int lsh_u64_one_by_width_minus_one_n = ((operator<<(uint64_t(1), uint64_t(63))) == uint64_t(0x8000000000000000ull)) ? 1 : 0;
static const int lsh_u64_all_ones_by_four_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) << uint64_t(4)) == uint64_t(0xFFFFFFFFFFFFFFF0ull)) ? 1 : 0;
static const int lsh_u64_all_ones_by_four_n = ((operator<<(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(4))) == uint64_t(0xFFFFFFFFFFFFFFF0ull)) ? 1 : 0;
static const int lsh_u64_max_by_zero_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) << uint64_t(0)) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int lsh_u64_max_by_zero_n = ((operator<<(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0))) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int lsh_u64_one_by_one_b = ((uint64_t(1) << uint64_t(1)) == uint64_t(2)) ? 1 : 0;
static const int lsh_u64_one_by_one_n = ((operator<<(uint64_t(1), uint64_t(1))) == uint64_t(2)) ? 1 : 0;
static const int lsh_u64_low_half_by_half_width_b = ((uint64_t(0x00000000FFFFFFFFull) << uint64_t(32)) == uint64_t(0xFFFFFFFF00000000ull)) ? 1 : 0;
static const int lsh_u64_low_half_by_half_width_n = ((operator<<(uint64_t(0x00000000FFFFFFFFull), uint64_t(32))) == uint64_t(0xFFFFFFFF00000000ull)) ? 1 : 0;

void check_lsh()
{
    switch (0) { case 0: case lsh_i8_one_by_width_minus_one_b: break; }
    switch (0) { case 0: case lsh_i8_one_by_width_minus_one_n: break; }
    switch (0) { case 0: case lsh_i8_all_ones_by_four_b: break; }
    switch (0) { case 0: case lsh_i8_all_ones_by_four_n: break; }
    switch (0) { case 0: case lsh_i8_max_by_zero_b: break; }
    switch (0) { case 0: case lsh_i8_max_by_zero_n: break; }
    switch (0) { case 0: case lsh_i8_one_by_one_b: break; }
    switch (0) { case 0: case lsh_i8_one_by_one_n: break; }
    switch (0) { case 0: case lsh_i8_low_half_by_half_width_b: break; }
    switch (0) { case 0: case lsh_i8_low_half_by_half_width_n: break; }
    switch (0) { case 0: case lsh_i8_minus_one_by_one_b: break; }
    switch (0) { case 0: case lsh_i8_minus_one_by_one_n: break; }
    switch (0) { case 0: case lsh_i8_max_by_one_b: break; }
    switch (0) { case 0: case lsh_i8_max_by_one_n: break; }
    switch (0) { case 0: case lsh_u8_one_by_width_minus_one_b: break; }
    switch (0) { case 0: case lsh_u8_one_by_width_minus_one_n: break; }
    switch (0) { case 0: case lsh_u8_all_ones_by_four_b: break; }
    switch (0) { case 0: case lsh_u8_all_ones_by_four_n: break; }
    switch (0) { case 0: case lsh_u8_max_by_zero_b: break; }
    switch (0) { case 0: case lsh_u8_max_by_zero_n: break; }
    switch (0) { case 0: case lsh_u8_one_by_one_b: break; }
    switch (0) { case 0: case lsh_u8_one_by_one_n: break; }
    switch (0) { case 0: case lsh_u8_low_half_by_half_width_b: break; }
    switch (0) { case 0: case lsh_u8_low_half_by_half_width_n: break; }
    switch (0) { case 0: case lsh_i64_one_by_width_minus_one_b: break; }
    switch (0) { case 0: case lsh_i64_one_by_width_minus_one_n: break; }
    switch (0) { case 0: case lsh_i64_all_ones_by_four_b: break; }
    switch (0) { case 0: case lsh_i64_all_ones_by_four_n: break; }
    switch (0) { case 0: case lsh_i64_max_by_zero_b: break; }
    switch (0) { case 0: case lsh_i64_max_by_zero_n: break; }
    switch (0) { case 0: case lsh_i64_one_by_one_b: break; }
    switch (0) { case 0: case lsh_i64_one_by_one_n: break; }
    switch (0) { case 0: case lsh_i64_low_half_by_half_width_b: break; }
    switch (0) { case 0: case lsh_i64_low_half_by_half_width_n: break; }
    switch (0) { case 0: case lsh_i64_minus_one_by_one_b: break; }
    switch (0) { case 0: case lsh_i64_minus_one_by_one_n: break; }
    switch (0) { case 0: case lsh_i64_max_by_one_b: break; }
    switch (0) { case 0: case lsh_i64_max_by_one_n: break; }
    switch (0) { case 0: case lsh_u64_one_by_width_minus_one_b: break; }
    switch (0) { case 0: case lsh_u64_one_by_width_minus_one_n: break; }
    switch (0) { case 0: case lsh_u64_all_ones_by_four_b: break; }
    switch (0) { case 0: case lsh_u64_all_ones_by_four_n: break; }
    switch (0) { case 0: case lsh_u64_max_by_zero_b: break; }
    switch (0) { case 0: case lsh_u64_max_by_zero_n: break; }
    switch (0) { case 0: case lsh_u64_one_by_one_b: break; }
    switch (0) { case 0: case lsh_u64_one_by_one_n: break; }
    switch (0) { case 0: case lsh_u64_low_half_by_half_width_b: break; }
    switch (0) { case 0: case lsh_u64_low_half_by_half_width_n: break; }
}

// ------------------------------------------------------------------------------------------------
// rsh `>>`
// Signed operands shift arithmetically and unsigned operands logically.
// ------------------------------------------------------------------------------------------------

static const int rsh_i8_top_bit_by_width_minus_one_b = ((int8_t(-128) >> int8_t(7)) == int8_t(-1)) ? 1 : 0;
static const int rsh_i8_top_bit_by_width_minus_one_n = ((operator>>(int8_t(-128), int8_t(7))) == int8_t(-1)) ? 1 : 0;
static const int rsh_i8_max_by_one_b = ((int8_t(127) >> int8_t(1)) == int8_t(63)) ? 1 : 0;
static const int rsh_i8_max_by_one_n = ((operator>>(int8_t(127), int8_t(1))) == int8_t(63)) ? 1 : 0;
static const int rsh_i8_max_by_zero_b = ((int8_t(127) >> int8_t(0)) == int8_t(127)) ? 1 : 0;
static const int rsh_i8_max_by_zero_n = ((operator>>(int8_t(127), int8_t(0))) == int8_t(127)) ? 1 : 0;
static const int rsh_i8_top_bit_by_one_b = ((int8_t(-128) >> int8_t(1)) == int8_t(-64)) ? 1 : 0;
static const int rsh_i8_top_bit_by_one_n = ((operator>>(int8_t(-128), int8_t(1))) == int8_t(-64)) ? 1 : 0;
static const int rsh_i8_all_ones_by_half_width_b = ((int8_t(-1) >> int8_t(4)) == int8_t(-1)) ? 1 : 0;
static const int rsh_i8_all_ones_by_half_width_n = ((operator>>(int8_t(-1), int8_t(4))) == int8_t(-1)) ? 1 : 0;
static const int rsh_i8_minus_eight_by_one_b = ((int8_t(-8) >> int8_t(1)) == int8_t(-4)) ? 1 : 0;
static const int rsh_i8_minus_eight_by_one_n = ((operator>>(int8_t(-8), int8_t(1))) == int8_t(-4)) ? 1 : 0;
static const int rsh_i8_minus_one_by_one_b = ((int8_t(-1) >> int8_t(1)) == int8_t(-1)) ? 1 : 0;
static const int rsh_i8_minus_one_by_one_n = ((operator>>(int8_t(-1), int8_t(1))) == int8_t(-1)) ? 1 : 0;
static const int rsh_i8_minus_one_by_width_minus_one_b = ((int8_t(-1) >> int8_t(7)) == int8_t(-1)) ? 1 : 0;
static const int rsh_i8_minus_one_by_width_minus_one_n = ((operator>>(int8_t(-1), int8_t(7))) == int8_t(-1)) ? 1 : 0;
static const int rsh_u8_top_bit_by_width_minus_one_b = ((uint8_t(0x80) >> uint8_t(0x07)) == uint8_t(0x01)) ? 1 : 0;
static const int rsh_u8_top_bit_by_width_minus_one_n = ((operator>>(uint8_t(0x80), uint8_t(0x07))) == uint8_t(0x01)) ? 1 : 0;
static const int rsh_u8_max_by_one_b = ((uint8_t(0xFF) >> uint8_t(0x01)) == uint8_t(0x7F)) ? 1 : 0;
static const int rsh_u8_max_by_one_n = ((operator>>(uint8_t(0xFF), uint8_t(0x01))) == uint8_t(0x7F)) ? 1 : 0;
static const int rsh_u8_max_by_zero_b = ((uint8_t(0xFF) >> uint8_t(0x00)) == uint8_t(0xFF)) ? 1 : 0;
static const int rsh_u8_max_by_zero_n = ((operator>>(uint8_t(0xFF), uint8_t(0x00))) == uint8_t(0xFF)) ? 1 : 0;
static const int rsh_u8_top_bit_by_one_b = ((uint8_t(0x80) >> uint8_t(0x01)) == uint8_t(0x40)) ? 1 : 0;
static const int rsh_u8_top_bit_by_one_n = ((operator>>(uint8_t(0x80), uint8_t(0x01))) == uint8_t(0x40)) ? 1 : 0;
static const int rsh_u8_all_ones_by_half_width_b = ((uint8_t(0xFF) >> uint8_t(0x04)) == uint8_t(0x0F)) ? 1 : 0;
static const int rsh_u8_all_ones_by_half_width_n = ((operator>>(uint8_t(0xFF), uint8_t(0x04))) == uint8_t(0x0F)) ? 1 : 0;
static const int rsh_i64_top_bit_by_width_minus_one_b = ((int64_t(0x8000000000000000ull) >> int64_t(63)) == int64_t(-1)) ? 1 : 0;
static const int rsh_i64_top_bit_by_width_minus_one_n = ((operator>>(int64_t(0x8000000000000000ull), int64_t(63))) == int64_t(-1)) ? 1 : 0;
static const int rsh_i64_max_by_one_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) >> int64_t(1)) == int64_t(0x3FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int rsh_i64_max_by_one_n = ((operator>>(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(1))) == int64_t(0x3FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int rsh_i64_max_by_zero_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) >> int64_t(0)) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int rsh_i64_max_by_zero_n = ((operator>>(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0))) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int rsh_i64_top_bit_by_one_b = ((int64_t(0x8000000000000000ull) >> int64_t(1)) == int64_t(0xC000000000000000ull)) ? 1 : 0;
static const int rsh_i64_top_bit_by_one_n = ((operator>>(int64_t(0x8000000000000000ull), int64_t(1))) == int64_t(0xC000000000000000ull)) ? 1 : 0;
static const int rsh_i64_all_ones_by_half_width_b = ((int64_t(-1) >> int64_t(32)) == int64_t(-1)) ? 1 : 0;
static const int rsh_i64_all_ones_by_half_width_n = ((operator>>(int64_t(-1), int64_t(32))) == int64_t(-1)) ? 1 : 0;
static const int rsh_i64_minus_eight_by_one_b = ((int64_t(-8) >> int64_t(1)) == int64_t(-4)) ? 1 : 0;
static const int rsh_i64_minus_eight_by_one_n = ((operator>>(int64_t(-8), int64_t(1))) == int64_t(-4)) ? 1 : 0;
static const int rsh_i64_minus_one_by_one_b = ((int64_t(-1) >> int64_t(1)) == int64_t(-1)) ? 1 : 0;
static const int rsh_i64_minus_one_by_one_n = ((operator>>(int64_t(-1), int64_t(1))) == int64_t(-1)) ? 1 : 0;
static const int rsh_i64_minus_one_by_width_minus_one_b = ((int64_t(-1) >> int64_t(63)) == int64_t(-1)) ? 1 : 0;
static const int rsh_i64_minus_one_by_width_minus_one_n = ((operator>>(int64_t(-1), int64_t(63))) == int64_t(-1)) ? 1 : 0;
static const int rsh_u64_top_bit_by_width_minus_one_b = ((uint64_t(0x8000000000000000ull) >> uint64_t(63)) == uint64_t(1)) ? 1 : 0;
static const int rsh_u64_top_bit_by_width_minus_one_n = ((operator>>(uint64_t(0x8000000000000000ull), uint64_t(63))) == uint64_t(1)) ? 1 : 0;
static const int rsh_u64_max_by_one_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) >> uint64_t(1)) == uint64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int rsh_u64_max_by_one_n = ((operator>>(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(1))) == uint64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int rsh_u64_max_by_zero_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) >> uint64_t(0)) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int rsh_u64_max_by_zero_n = ((operator>>(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0))) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int rsh_u64_top_bit_by_one_b = ((uint64_t(0x8000000000000000ull) >> uint64_t(1)) == uint64_t(0x4000000000000000ull)) ? 1 : 0;
static const int rsh_u64_top_bit_by_one_n = ((operator>>(uint64_t(0x8000000000000000ull), uint64_t(1))) == uint64_t(0x4000000000000000ull)) ? 1 : 0;
static const int rsh_u64_all_ones_by_half_width_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) >> uint64_t(32)) == uint64_t(0x00000000FFFFFFFFull)) ? 1 : 0;
static const int rsh_u64_all_ones_by_half_width_n = ((operator>>(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(32))) == uint64_t(0x00000000FFFFFFFFull)) ? 1 : 0;

void check_rsh()
{
    switch (0) { case 0: case rsh_i8_top_bit_by_width_minus_one_b: break; }
    switch (0) { case 0: case rsh_i8_top_bit_by_width_minus_one_n: break; }
    switch (0) { case 0: case rsh_i8_max_by_one_b: break; }
    switch (0) { case 0: case rsh_i8_max_by_one_n: break; }
    switch (0) { case 0: case rsh_i8_max_by_zero_b: break; }
    switch (0) { case 0: case rsh_i8_max_by_zero_n: break; }
    switch (0) { case 0: case rsh_i8_top_bit_by_one_b: break; }
    switch (0) { case 0: case rsh_i8_top_bit_by_one_n: break; }
    switch (0) { case 0: case rsh_i8_all_ones_by_half_width_b: break; }
    switch (0) { case 0: case rsh_i8_all_ones_by_half_width_n: break; }
    switch (0) { case 0: case rsh_i8_minus_eight_by_one_b: break; }
    switch (0) { case 0: case rsh_i8_minus_eight_by_one_n: break; }
    switch (0) { case 0: case rsh_i8_minus_one_by_one_b: break; }
    switch (0) { case 0: case rsh_i8_minus_one_by_one_n: break; }
    switch (0) { case 0: case rsh_i8_minus_one_by_width_minus_one_b: break; }
    switch (0) { case 0: case rsh_i8_minus_one_by_width_minus_one_n: break; }
    switch (0) { case 0: case rsh_u8_top_bit_by_width_minus_one_b: break; }
    switch (0) { case 0: case rsh_u8_top_bit_by_width_minus_one_n: break; }
    switch (0) { case 0: case rsh_u8_max_by_one_b: break; }
    switch (0) { case 0: case rsh_u8_max_by_one_n: break; }
    switch (0) { case 0: case rsh_u8_max_by_zero_b: break; }
    switch (0) { case 0: case rsh_u8_max_by_zero_n: break; }
    switch (0) { case 0: case rsh_u8_top_bit_by_one_b: break; }
    switch (0) { case 0: case rsh_u8_top_bit_by_one_n: break; }
    switch (0) { case 0: case rsh_u8_all_ones_by_half_width_b: break; }
    switch (0) { case 0: case rsh_u8_all_ones_by_half_width_n: break; }
    switch (0) { case 0: case rsh_i64_top_bit_by_width_minus_one_b: break; }
    switch (0) { case 0: case rsh_i64_top_bit_by_width_minus_one_n: break; }
    switch (0) { case 0: case rsh_i64_max_by_one_b: break; }
    switch (0) { case 0: case rsh_i64_max_by_one_n: break; }
    switch (0) { case 0: case rsh_i64_max_by_zero_b: break; }
    switch (0) { case 0: case rsh_i64_max_by_zero_n: break; }
    switch (0) { case 0: case rsh_i64_top_bit_by_one_b: break; }
    switch (0) { case 0: case rsh_i64_top_bit_by_one_n: break; }
    switch (0) { case 0: case rsh_i64_all_ones_by_half_width_b: break; }
    switch (0) { case 0: case rsh_i64_all_ones_by_half_width_n: break; }
    switch (0) { case 0: case rsh_i64_minus_eight_by_one_b: break; }
    switch (0) { case 0: case rsh_i64_minus_eight_by_one_n: break; }
    switch (0) { case 0: case rsh_i64_minus_one_by_one_b: break; }
    switch (0) { case 0: case rsh_i64_minus_one_by_one_n: break; }
    switch (0) { case 0: case rsh_i64_minus_one_by_width_minus_one_b: break; }
    switch (0) { case 0: case rsh_i64_minus_one_by_width_minus_one_n: break; }
    switch (0) { case 0: case rsh_u64_top_bit_by_width_minus_one_b: break; }
    switch (0) { case 0: case rsh_u64_top_bit_by_width_minus_one_n: break; }
    switch (0) { case 0: case rsh_u64_max_by_one_b: break; }
    switch (0) { case 0: case rsh_u64_max_by_one_n: break; }
    switch (0) { case 0: case rsh_u64_max_by_zero_b: break; }
    switch (0) { case 0: case rsh_u64_max_by_zero_n: break; }
    switch (0) { case 0: case rsh_u64_top_bit_by_one_b: break; }
    switch (0) { case 0: case rsh_u64_top_bit_by_one_n: break; }
    switch (0) { case 0: case rsh_u64_all_ones_by_half_width_b: break; }
    switch (0) { case 0: case rsh_u64_all_ones_by_half_width_n: break; }
}
