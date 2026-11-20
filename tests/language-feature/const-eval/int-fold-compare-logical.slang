//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

// Front-end constant folding of the comparison operators `== != < > <= >=` on int8_t,
// uint8_t, int64_t and uint64_t operands, and of the logical operators `! && ||` and the
// conditional operator `?:`.
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
// eql `==`
// ------------------------------------------------------------------------------------------------

static const int eql_i8_min_min_b = ((int8_t(-128) == int8_t(-128)) == true) ? 1 : 0;
static const int eql_i8_min_min_n = ((operator==(int8_t(-128), int8_t(-128))) == true) ? 1 : 0;
static const int eql_i8_min_max_b = ((int8_t(-128) == int8_t(127)) == false) ? 1 : 0;
static const int eql_i8_min_max_n = ((operator==(int8_t(-128), int8_t(127))) == false) ? 1 : 0;
static const int eql_i8_top_bit_zero_b = ((int8_t(-128) == int8_t(0)) == false) ? 1 : 0;
static const int eql_i8_top_bit_zero_n = ((operator==(int8_t(-128), int8_t(0))) == false) ? 1 : 0;
static const int eql_i8_high_half_zero_b = ((int8_t(-16) == int8_t(0)) == false) ? 1 : 0;
static const int eql_i8_high_half_zero_n = ((operator==(int8_t(-16), int8_t(0))) == false) ? 1 : 0;
static const int eql_i8_max_max_b = ((int8_t(127) == int8_t(127)) == true) ? 1 : 0;
static const int eql_i8_max_max_n = ((operator==(int8_t(127), int8_t(127))) == true) ? 1 : 0;
static const int eql_u8_min_min_b = ((uint8_t(0x00) == uint8_t(0x00)) == true) ? 1 : 0;
static const int eql_u8_min_min_n = ((operator==(uint8_t(0x00), uint8_t(0x00))) == true) ? 1 : 0;
static const int eql_u8_min_max_b = ((uint8_t(0x00) == uint8_t(0xFF)) == false) ? 1 : 0;
static const int eql_u8_min_max_n = ((operator==(uint8_t(0x00), uint8_t(0xFF))) == false) ? 1 : 0;
static const int eql_u8_top_bit_zero_b = ((uint8_t(0x80) == uint8_t(0x00)) == false) ? 1 : 0;
static const int eql_u8_top_bit_zero_n = ((operator==(uint8_t(0x80), uint8_t(0x00))) == false) ? 1 : 0;
static const int eql_u8_high_half_zero_b = ((uint8_t(0xF0) == uint8_t(0x00)) == false) ? 1 : 0;
static const int eql_u8_high_half_zero_n = ((operator==(uint8_t(0xF0), uint8_t(0x00))) == false) ? 1 : 0;
static const int eql_u8_max_max_b = ((uint8_t(0xFF) == uint8_t(0xFF)) == true) ? 1 : 0;
static const int eql_u8_max_max_n = ((operator==(uint8_t(0xFF), uint8_t(0xFF))) == true) ? 1 : 0;
static const int eql_i64_min_min_b = ((int64_t(0x8000000000000000ull) == int64_t(0x8000000000000000ull)) == true) ? 1 : 0;
static const int eql_i64_min_min_n = ((operator==(int64_t(0x8000000000000000ull), int64_t(0x8000000000000000ull))) == true) ? 1 : 0;
static const int eql_i64_min_max_b = ((int64_t(0x8000000000000000ull) == int64_t(0x7FFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int eql_i64_min_max_n = ((operator==(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == false) ? 1 : 0;
static const int eql_i64_top_bit_zero_b = ((int64_t(0x8000000000000000ull) == int64_t(0)) == false) ? 1 : 0;
static const int eql_i64_top_bit_zero_n = ((operator==(int64_t(0x8000000000000000ull), int64_t(0))) == false) ? 1 : 0;
static const int eql_i64_high_half_zero_b = ((int64_t(0xFFFFFFFF00000000ull) == int64_t(0)) == false) ? 1 : 0;
static const int eql_i64_high_half_zero_n = ((operator==(int64_t(0xFFFFFFFF00000000ull), int64_t(0))) == false) ? 1 : 0;
static const int eql_i64_max_max_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) == int64_t(0x7FFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int eql_i64_max_max_n = ((operator==(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0x7FFFFFFFFFFFFFFFull))) == true) ? 1 : 0;
static const int eql_u64_min_min_b = ((uint64_t(0) == uint64_t(0)) == true) ? 1 : 0;
static const int eql_u64_min_min_n = ((operator==(uint64_t(0), uint64_t(0))) == true) ? 1 : 0;
static const int eql_u64_min_max_b = ((uint64_t(0) == uint64_t(0xFFFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int eql_u64_min_max_n = ((operator==(uint64_t(0), uint64_t(0xFFFFFFFFFFFFFFFFull))) == false) ? 1 : 0;
static const int eql_u64_top_bit_zero_b = ((uint64_t(0x8000000000000000ull) == uint64_t(0)) == false) ? 1 : 0;
static const int eql_u64_top_bit_zero_n = ((operator==(uint64_t(0x8000000000000000ull), uint64_t(0))) == false) ? 1 : 0;
static const int eql_u64_high_half_zero_b = ((uint64_t(0xFFFFFFFF00000000ull) == uint64_t(0)) == false) ? 1 : 0;
static const int eql_u64_high_half_zero_n = ((operator==(uint64_t(0xFFFFFFFF00000000ull), uint64_t(0))) == false) ? 1 : 0;
static const int eql_u64_max_max_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) == uint64_t(0xFFFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int eql_u64_max_max_n = ((operator==(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0xFFFFFFFFFFFFFFFFull))) == true) ? 1 : 0;

void check_eql()
{
    switch (0) { case 0: case eql_i8_min_min_b: break; }
    switch (0) { case 0: case eql_i8_min_min_n: break; }
    switch (0) { case 0: case eql_i8_min_max_b: break; }
    switch (0) { case 0: case eql_i8_min_max_n: break; }
    switch (0) { case 0: case eql_i8_top_bit_zero_b: break; }
    switch (0) { case 0: case eql_i8_top_bit_zero_n: break; }
    switch (0) { case 0: case eql_i8_high_half_zero_b: break; }
    switch (0) { case 0: case eql_i8_high_half_zero_n: break; }
    switch (0) { case 0: case eql_i8_max_max_b: break; }
    switch (0) { case 0: case eql_i8_max_max_n: break; }
    switch (0) { case 0: case eql_u8_min_min_b: break; }
    switch (0) { case 0: case eql_u8_min_min_n: break; }
    switch (0) { case 0: case eql_u8_min_max_b: break; }
    switch (0) { case 0: case eql_u8_min_max_n: break; }
    switch (0) { case 0: case eql_u8_top_bit_zero_b: break; }
    switch (0) { case 0: case eql_u8_top_bit_zero_n: break; }
    switch (0) { case 0: case eql_u8_high_half_zero_b: break; }
    switch (0) { case 0: case eql_u8_high_half_zero_n: break; }
    switch (0) { case 0: case eql_u8_max_max_b: break; }
    switch (0) { case 0: case eql_u8_max_max_n: break; }
    switch (0) { case 0: case eql_i64_min_min_b: break; }
    switch (0) { case 0: case eql_i64_min_min_n: break; }
    switch (0) { case 0: case eql_i64_min_max_b: break; }
    switch (0) { case 0: case eql_i64_min_max_n: break; }
    switch (0) { case 0: case eql_i64_top_bit_zero_b: break; }
    switch (0) { case 0: case eql_i64_top_bit_zero_n: break; }
    switch (0) { case 0: case eql_i64_high_half_zero_b: break; }
    switch (0) { case 0: case eql_i64_high_half_zero_n: break; }
    switch (0) { case 0: case eql_i64_max_max_b: break; }
    switch (0) { case 0: case eql_i64_max_max_n: break; }
    switch (0) { case 0: case eql_u64_min_min_b: break; }
    switch (0) { case 0: case eql_u64_min_min_n: break; }
    switch (0) { case 0: case eql_u64_min_max_b: break; }
    switch (0) { case 0: case eql_u64_min_max_n: break; }
    switch (0) { case 0: case eql_u64_top_bit_zero_b: break; }
    switch (0) { case 0: case eql_u64_top_bit_zero_n: break; }
    switch (0) { case 0: case eql_u64_high_half_zero_b: break; }
    switch (0) { case 0: case eql_u64_high_half_zero_n: break; }
    switch (0) { case 0: case eql_u64_max_max_b: break; }
    switch (0) { case 0: case eql_u64_max_max_n: break; }
}

// ------------------------------------------------------------------------------------------------
// neq `!=`
// ------------------------------------------------------------------------------------------------

static const int neq_i8_min_min_b = ((int8_t(-128) != int8_t(-128)) == false) ? 1 : 0;
static const int neq_i8_min_min_n = ((operator!=(int8_t(-128), int8_t(-128))) == false) ? 1 : 0;
static const int neq_i8_min_max_b = ((int8_t(-128) != int8_t(127)) == true) ? 1 : 0;
static const int neq_i8_min_max_n = ((operator!=(int8_t(-128), int8_t(127))) == true) ? 1 : 0;
static const int neq_i8_top_bit_zero_b = ((int8_t(-128) != int8_t(0)) == true) ? 1 : 0;
static const int neq_i8_top_bit_zero_n = ((operator!=(int8_t(-128), int8_t(0))) == true) ? 1 : 0;
static const int neq_i8_high_half_zero_b = ((int8_t(-16) != int8_t(0)) == true) ? 1 : 0;
static const int neq_i8_high_half_zero_n = ((operator!=(int8_t(-16), int8_t(0))) == true) ? 1 : 0;
static const int neq_i8_max_max_b = ((int8_t(127) != int8_t(127)) == false) ? 1 : 0;
static const int neq_i8_max_max_n = ((operator!=(int8_t(127), int8_t(127))) == false) ? 1 : 0;
static const int neq_u8_min_min_b = ((uint8_t(0x00) != uint8_t(0x00)) == false) ? 1 : 0;
static const int neq_u8_min_min_n = ((operator!=(uint8_t(0x00), uint8_t(0x00))) == false) ? 1 : 0;
static const int neq_u8_min_max_b = ((uint8_t(0x00) != uint8_t(0xFF)) == true) ? 1 : 0;
static const int neq_u8_min_max_n = ((operator!=(uint8_t(0x00), uint8_t(0xFF))) == true) ? 1 : 0;
static const int neq_u8_top_bit_zero_b = ((uint8_t(0x80) != uint8_t(0x00)) == true) ? 1 : 0;
static const int neq_u8_top_bit_zero_n = ((operator!=(uint8_t(0x80), uint8_t(0x00))) == true) ? 1 : 0;
static const int neq_u8_high_half_zero_b = ((uint8_t(0xF0) != uint8_t(0x00)) == true) ? 1 : 0;
static const int neq_u8_high_half_zero_n = ((operator!=(uint8_t(0xF0), uint8_t(0x00))) == true) ? 1 : 0;
static const int neq_u8_max_max_b = ((uint8_t(0xFF) != uint8_t(0xFF)) == false) ? 1 : 0;
static const int neq_u8_max_max_n = ((operator!=(uint8_t(0xFF), uint8_t(0xFF))) == false) ? 1 : 0;
static const int neq_i64_min_min_b = ((int64_t(0x8000000000000000ull) != int64_t(0x8000000000000000ull)) == false) ? 1 : 0;
static const int neq_i64_min_min_n = ((operator!=(int64_t(0x8000000000000000ull), int64_t(0x8000000000000000ull))) == false) ? 1 : 0;
static const int neq_i64_min_max_b = ((int64_t(0x8000000000000000ull) != int64_t(0x7FFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int neq_i64_min_max_n = ((operator!=(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == true) ? 1 : 0;
static const int neq_i64_top_bit_zero_b = ((int64_t(0x8000000000000000ull) != int64_t(0)) == true) ? 1 : 0;
static const int neq_i64_top_bit_zero_n = ((operator!=(int64_t(0x8000000000000000ull), int64_t(0))) == true) ? 1 : 0;
static const int neq_i64_high_half_zero_b = ((int64_t(0xFFFFFFFF00000000ull) != int64_t(0)) == true) ? 1 : 0;
static const int neq_i64_high_half_zero_n = ((operator!=(int64_t(0xFFFFFFFF00000000ull), int64_t(0))) == true) ? 1 : 0;
static const int neq_i64_max_max_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) != int64_t(0x7FFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int neq_i64_max_max_n = ((operator!=(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0x7FFFFFFFFFFFFFFFull))) == false) ? 1 : 0;
static const int neq_u64_min_min_b = ((uint64_t(0) != uint64_t(0)) == false) ? 1 : 0;
static const int neq_u64_min_min_n = ((operator!=(uint64_t(0), uint64_t(0))) == false) ? 1 : 0;
static const int neq_u64_min_max_b = ((uint64_t(0) != uint64_t(0xFFFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int neq_u64_min_max_n = ((operator!=(uint64_t(0), uint64_t(0xFFFFFFFFFFFFFFFFull))) == true) ? 1 : 0;
static const int neq_u64_top_bit_zero_b = ((uint64_t(0x8000000000000000ull) != uint64_t(0)) == true) ? 1 : 0;
static const int neq_u64_top_bit_zero_n = ((operator!=(uint64_t(0x8000000000000000ull), uint64_t(0))) == true) ? 1 : 0;
static const int neq_u64_high_half_zero_b = ((uint64_t(0xFFFFFFFF00000000ull) != uint64_t(0)) == true) ? 1 : 0;
static const int neq_u64_high_half_zero_n = ((operator!=(uint64_t(0xFFFFFFFF00000000ull), uint64_t(0))) == true) ? 1 : 0;
static const int neq_u64_max_max_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) != uint64_t(0xFFFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int neq_u64_max_max_n = ((operator!=(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0xFFFFFFFFFFFFFFFFull))) == false) ? 1 : 0;

void check_neq()
{
    switch (0) { case 0: case neq_i8_min_min_b: break; }
    switch (0) { case 0: case neq_i8_min_min_n: break; }
    switch (0) { case 0: case neq_i8_min_max_b: break; }
    switch (0) { case 0: case neq_i8_min_max_n: break; }
    switch (0) { case 0: case neq_i8_top_bit_zero_b: break; }
    switch (0) { case 0: case neq_i8_top_bit_zero_n: break; }
    switch (0) { case 0: case neq_i8_high_half_zero_b: break; }
    switch (0) { case 0: case neq_i8_high_half_zero_n: break; }
    switch (0) { case 0: case neq_i8_max_max_b: break; }
    switch (0) { case 0: case neq_i8_max_max_n: break; }
    switch (0) { case 0: case neq_u8_min_min_b: break; }
    switch (0) { case 0: case neq_u8_min_min_n: break; }
    switch (0) { case 0: case neq_u8_min_max_b: break; }
    switch (0) { case 0: case neq_u8_min_max_n: break; }
    switch (0) { case 0: case neq_u8_top_bit_zero_b: break; }
    switch (0) { case 0: case neq_u8_top_bit_zero_n: break; }
    switch (0) { case 0: case neq_u8_high_half_zero_b: break; }
    switch (0) { case 0: case neq_u8_high_half_zero_n: break; }
    switch (0) { case 0: case neq_u8_max_max_b: break; }
    switch (0) { case 0: case neq_u8_max_max_n: break; }
    switch (0) { case 0: case neq_i64_min_min_b: break; }
    switch (0) { case 0: case neq_i64_min_min_n: break; }
    switch (0) { case 0: case neq_i64_min_max_b: break; }
    switch (0) { case 0: case neq_i64_min_max_n: break; }
    switch (0) { case 0: case neq_i64_top_bit_zero_b: break; }
    switch (0) { case 0: case neq_i64_top_bit_zero_n: break; }
    switch (0) { case 0: case neq_i64_high_half_zero_b: break; }
    switch (0) { case 0: case neq_i64_high_half_zero_n: break; }
    switch (0) { case 0: case neq_i64_max_max_b: break; }
    switch (0) { case 0: case neq_i64_max_max_n: break; }
    switch (0) { case 0: case neq_u64_min_min_b: break; }
    switch (0) { case 0: case neq_u64_min_min_n: break; }
    switch (0) { case 0: case neq_u64_min_max_b: break; }
    switch (0) { case 0: case neq_u64_min_max_n: break; }
    switch (0) { case 0: case neq_u64_top_bit_zero_b: break; }
    switch (0) { case 0: case neq_u64_top_bit_zero_n: break; }
    switch (0) { case 0: case neq_u64_high_half_zero_b: break; }
    switch (0) { case 0: case neq_u64_high_half_zero_n: break; }
    switch (0) { case 0: case neq_u64_max_max_b: break; }
    switch (0) { case 0: case neq_u64_max_max_n: break; }
}

// ------------------------------------------------------------------------------------------------
// less `<`
// ------------------------------------------------------------------------------------------------

static const int less_i8_min_max_b = ((int8_t(-128) < int8_t(127)) == true) ? 1 : 0;
static const int less_i8_min_max_n = ((operator<(int8_t(-128), int8_t(127))) == true) ? 1 : 0;
static const int less_i8_max_min_b = ((int8_t(127) < int8_t(-128)) == false) ? 1 : 0;
static const int less_i8_max_min_n = ((operator<(int8_t(127), int8_t(-128))) == false) ? 1 : 0;
static const int less_i8_five_five_b = ((int8_t(5) < int8_t(5)) == false) ? 1 : 0;
static const int less_i8_five_five_n = ((operator<(int8_t(5), int8_t(5))) == false) ? 1 : 0;
static const int less_i8_zero_max_b = ((int8_t(0) < int8_t(127)) == true) ? 1 : 0;
static const int less_i8_zero_max_n = ((operator<(int8_t(0), int8_t(127))) == true) ? 1 : 0;
static const int less_i8_max_five_b = ((int8_t(127) < int8_t(5)) == false) ? 1 : 0;
static const int less_i8_max_five_n = ((operator<(int8_t(127), int8_t(5))) == false) ? 1 : 0;
static const int less_i8_minus_one_zero_b = ((int8_t(-1) < int8_t(0)) == true) ? 1 : 0;
static const int less_i8_minus_one_zero_n = ((operator<(int8_t(-1), int8_t(0))) == true) ? 1 : 0;
static const int less_i8_zero_minus_one_b = ((int8_t(0) < int8_t(-1)) == false) ? 1 : 0;
static const int less_i8_zero_minus_one_n = ((operator<(int8_t(0), int8_t(-1))) == false) ? 1 : 0;
static const int less_u8_min_max_b = ((uint8_t(0x00) < uint8_t(0xFF)) == true) ? 1 : 0;
static const int less_u8_min_max_n = ((operator<(uint8_t(0x00), uint8_t(0xFF))) == true) ? 1 : 0;
static const int less_u8_max_min_b = ((uint8_t(0xFF) < uint8_t(0x00)) == false) ? 1 : 0;
static const int less_u8_max_min_n = ((operator<(uint8_t(0xFF), uint8_t(0x00))) == false) ? 1 : 0;
static const int less_u8_five_five_b = ((uint8_t(0x05) < uint8_t(0x05)) == false) ? 1 : 0;
static const int less_u8_five_five_n = ((operator<(uint8_t(0x05), uint8_t(0x05))) == false) ? 1 : 0;
static const int less_u8_max_five_b = ((uint8_t(0xFF) < uint8_t(0x05)) == false) ? 1 : 0;
static const int less_u8_max_five_n = ((operator<(uint8_t(0xFF), uint8_t(0x05))) == false) ? 1 : 0;
static const int less_u8_top_bit_vs_top_bit_minus_one_b = ((uint8_t(0x80) < uint8_t(0x7F)) == false) ? 1 : 0;
static const int less_u8_top_bit_vs_top_bit_minus_one_n = ((operator<(uint8_t(0x80), uint8_t(0x7F))) == false) ? 1 : 0;
static const int less_i64_min_max_b = ((int64_t(0x8000000000000000ull) < int64_t(0x7FFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int less_i64_min_max_n = ((operator<(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == true) ? 1 : 0;
static const int less_i64_max_min_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) < int64_t(0x8000000000000000ull)) == false) ? 1 : 0;
static const int less_i64_max_min_n = ((operator<(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0x8000000000000000ull))) == false) ? 1 : 0;
static const int less_i64_five_five_b = ((int64_t(5) < int64_t(5)) == false) ? 1 : 0;
static const int less_i64_five_five_n = ((operator<(int64_t(5), int64_t(5))) == false) ? 1 : 0;
static const int less_i64_zero_max_b = ((int64_t(0) < int64_t(0x7FFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int less_i64_zero_max_n = ((operator<(int64_t(0), int64_t(0x7FFFFFFFFFFFFFFFull))) == true) ? 1 : 0;
static const int less_i64_max_five_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) < int64_t(5)) == false) ? 1 : 0;
static const int less_i64_max_five_n = ((operator<(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(5))) == false) ? 1 : 0;
static const int less_i64_minus_one_zero_b = ((int64_t(-1) < int64_t(0)) == true) ? 1 : 0;
static const int less_i64_minus_one_zero_n = ((operator<(int64_t(-1), int64_t(0))) == true) ? 1 : 0;
static const int less_i64_zero_minus_one_b = ((int64_t(0) < int64_t(-1)) == false) ? 1 : 0;
static const int less_i64_zero_minus_one_n = ((operator<(int64_t(0), int64_t(-1))) == false) ? 1 : 0;
static const int less_u64_min_max_b = ((uint64_t(0) < uint64_t(0xFFFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int less_u64_min_max_n = ((operator<(uint64_t(0), uint64_t(0xFFFFFFFFFFFFFFFFull))) == true) ? 1 : 0;
static const int less_u64_max_min_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) < uint64_t(0)) == false) ? 1 : 0;
static const int less_u64_max_min_n = ((operator<(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0))) == false) ? 1 : 0;
static const int less_u64_five_five_b = ((uint64_t(5) < uint64_t(5)) == false) ? 1 : 0;
static const int less_u64_five_five_n = ((operator<(uint64_t(5), uint64_t(5))) == false) ? 1 : 0;
static const int less_u64_max_five_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) < uint64_t(5)) == false) ? 1 : 0;
static const int less_u64_max_five_n = ((operator<(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(5))) == false) ? 1 : 0;
static const int less_u64_top_bit_vs_top_bit_minus_one_b = ((uint64_t(0x8000000000000000ull) < uint64_t(0x7FFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int less_u64_top_bit_vs_top_bit_minus_one_n = ((operator<(uint64_t(0x8000000000000000ull), uint64_t(0x7FFFFFFFFFFFFFFFull))) == false) ? 1 : 0;

void check_less()
{
    switch (0) { case 0: case less_i8_min_max_b: break; }
    switch (0) { case 0: case less_i8_min_max_n: break; }
    switch (0) { case 0: case less_i8_max_min_b: break; }
    switch (0) { case 0: case less_i8_max_min_n: break; }
    switch (0) { case 0: case less_i8_five_five_b: break; }
    switch (0) { case 0: case less_i8_five_five_n: break; }
    switch (0) { case 0: case less_i8_zero_max_b: break; }
    switch (0) { case 0: case less_i8_zero_max_n: break; }
    switch (0) { case 0: case less_i8_max_five_b: break; }
    switch (0) { case 0: case less_i8_max_five_n: break; }
    switch (0) { case 0: case less_i8_minus_one_zero_b: break; }
    switch (0) { case 0: case less_i8_minus_one_zero_n: break; }
    switch (0) { case 0: case less_i8_zero_minus_one_b: break; }
    switch (0) { case 0: case less_i8_zero_minus_one_n: break; }
    switch (0) { case 0: case less_u8_min_max_b: break; }
    switch (0) { case 0: case less_u8_min_max_n: break; }
    switch (0) { case 0: case less_u8_max_min_b: break; }
    switch (0) { case 0: case less_u8_max_min_n: break; }
    switch (0) { case 0: case less_u8_five_five_b: break; }
    switch (0) { case 0: case less_u8_five_five_n: break; }
    switch (0) { case 0: case less_u8_max_five_b: break; }
    switch (0) { case 0: case less_u8_max_five_n: break; }
    switch (0) { case 0: case less_u8_top_bit_vs_top_bit_minus_one_b: break; }
    switch (0) { case 0: case less_u8_top_bit_vs_top_bit_minus_one_n: break; }
    switch (0) { case 0: case less_i64_min_max_b: break; }
    switch (0) { case 0: case less_i64_min_max_n: break; }
    switch (0) { case 0: case less_i64_max_min_b: break; }
    switch (0) { case 0: case less_i64_max_min_n: break; }
    switch (0) { case 0: case less_i64_five_five_b: break; }
    switch (0) { case 0: case less_i64_five_five_n: break; }
    switch (0) { case 0: case less_i64_zero_max_b: break; }
    switch (0) { case 0: case less_i64_zero_max_n: break; }
    switch (0) { case 0: case less_i64_max_five_b: break; }
    switch (0) { case 0: case less_i64_max_five_n: break; }
    switch (0) { case 0: case less_i64_minus_one_zero_b: break; }
    switch (0) { case 0: case less_i64_minus_one_zero_n: break; }
    switch (0) { case 0: case less_i64_zero_minus_one_b: break; }
    switch (0) { case 0: case less_i64_zero_minus_one_n: break; }
    switch (0) { case 0: case less_u64_min_max_b: break; }
    switch (0) { case 0: case less_u64_min_max_n: break; }
    switch (0) { case 0: case less_u64_max_min_b: break; }
    switch (0) { case 0: case less_u64_max_min_n: break; }
    switch (0) { case 0: case less_u64_five_five_b: break; }
    switch (0) { case 0: case less_u64_five_five_n: break; }
    switch (0) { case 0: case less_u64_max_five_b: break; }
    switch (0) { case 0: case less_u64_max_five_n: break; }
    switch (0) { case 0: case less_u64_top_bit_vs_top_bit_minus_one_b: break; }
    switch (0) { case 0: case less_u64_top_bit_vs_top_bit_minus_one_n: break; }
}

// ------------------------------------------------------------------------------------------------
// greater `>`
// ------------------------------------------------------------------------------------------------

static const int greater_i8_min_max_b = ((int8_t(-128) > int8_t(127)) == false) ? 1 : 0;
static const int greater_i8_min_max_n = ((operator>(int8_t(-128), int8_t(127))) == false) ? 1 : 0;
static const int greater_i8_max_min_b = ((int8_t(127) > int8_t(-128)) == true) ? 1 : 0;
static const int greater_i8_max_min_n = ((operator>(int8_t(127), int8_t(-128))) == true) ? 1 : 0;
static const int greater_i8_five_five_b = ((int8_t(5) > int8_t(5)) == false) ? 1 : 0;
static const int greater_i8_five_five_n = ((operator>(int8_t(5), int8_t(5))) == false) ? 1 : 0;
static const int greater_i8_zero_max_b = ((int8_t(0) > int8_t(127)) == false) ? 1 : 0;
static const int greater_i8_zero_max_n = ((operator>(int8_t(0), int8_t(127))) == false) ? 1 : 0;
static const int greater_i8_max_five_b = ((int8_t(127) > int8_t(5)) == true) ? 1 : 0;
static const int greater_i8_max_five_n = ((operator>(int8_t(127), int8_t(5))) == true) ? 1 : 0;
static const int greater_i8_minus_one_zero_b = ((int8_t(-1) > int8_t(0)) == false) ? 1 : 0;
static const int greater_i8_minus_one_zero_n = ((operator>(int8_t(-1), int8_t(0))) == false) ? 1 : 0;
static const int greater_i8_zero_minus_one_b = ((int8_t(0) > int8_t(-1)) == true) ? 1 : 0;
static const int greater_i8_zero_minus_one_n = ((operator>(int8_t(0), int8_t(-1))) == true) ? 1 : 0;
static const int greater_u8_min_max_b = ((uint8_t(0x00) > uint8_t(0xFF)) == false) ? 1 : 0;
static const int greater_u8_min_max_n = ((operator>(uint8_t(0x00), uint8_t(0xFF))) == false) ? 1 : 0;
static const int greater_u8_max_min_b = ((uint8_t(0xFF) > uint8_t(0x00)) == true) ? 1 : 0;
static const int greater_u8_max_min_n = ((operator>(uint8_t(0xFF), uint8_t(0x00))) == true) ? 1 : 0;
static const int greater_u8_five_five_b = ((uint8_t(0x05) > uint8_t(0x05)) == false) ? 1 : 0;
static const int greater_u8_five_five_n = ((operator>(uint8_t(0x05), uint8_t(0x05))) == false) ? 1 : 0;
static const int greater_u8_max_five_b = ((uint8_t(0xFF) > uint8_t(0x05)) == true) ? 1 : 0;
static const int greater_u8_max_five_n = ((operator>(uint8_t(0xFF), uint8_t(0x05))) == true) ? 1 : 0;
static const int greater_u8_top_bit_vs_top_bit_minus_one_b = ((uint8_t(0x80) > uint8_t(0x7F)) == true) ? 1 : 0;
static const int greater_u8_top_bit_vs_top_bit_minus_one_n = ((operator>(uint8_t(0x80), uint8_t(0x7F))) == true) ? 1 : 0;
static const int greater_i64_min_max_b = ((int64_t(0x8000000000000000ull) > int64_t(0x7FFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int greater_i64_min_max_n = ((operator>(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == false) ? 1 : 0;
static const int greater_i64_max_min_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) > int64_t(0x8000000000000000ull)) == true) ? 1 : 0;
static const int greater_i64_max_min_n = ((operator>(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0x8000000000000000ull))) == true) ? 1 : 0;
static const int greater_i64_five_five_b = ((int64_t(5) > int64_t(5)) == false) ? 1 : 0;
static const int greater_i64_five_five_n = ((operator>(int64_t(5), int64_t(5))) == false) ? 1 : 0;
static const int greater_i64_zero_max_b = ((int64_t(0) > int64_t(0x7FFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int greater_i64_zero_max_n = ((operator>(int64_t(0), int64_t(0x7FFFFFFFFFFFFFFFull))) == false) ? 1 : 0;
static const int greater_i64_max_five_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) > int64_t(5)) == true) ? 1 : 0;
static const int greater_i64_max_five_n = ((operator>(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(5))) == true) ? 1 : 0;
static const int greater_i64_minus_one_zero_b = ((int64_t(-1) > int64_t(0)) == false) ? 1 : 0;
static const int greater_i64_minus_one_zero_n = ((operator>(int64_t(-1), int64_t(0))) == false) ? 1 : 0;
static const int greater_i64_zero_minus_one_b = ((int64_t(0) > int64_t(-1)) == true) ? 1 : 0;
static const int greater_i64_zero_minus_one_n = ((operator>(int64_t(0), int64_t(-1))) == true) ? 1 : 0;
static const int greater_u64_min_max_b = ((uint64_t(0) > uint64_t(0xFFFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int greater_u64_min_max_n = ((operator>(uint64_t(0), uint64_t(0xFFFFFFFFFFFFFFFFull))) == false) ? 1 : 0;
static const int greater_u64_max_min_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) > uint64_t(0)) == true) ? 1 : 0;
static const int greater_u64_max_min_n = ((operator>(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0))) == true) ? 1 : 0;
static const int greater_u64_five_five_b = ((uint64_t(5) > uint64_t(5)) == false) ? 1 : 0;
static const int greater_u64_five_five_n = ((operator>(uint64_t(5), uint64_t(5))) == false) ? 1 : 0;
static const int greater_u64_max_five_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) > uint64_t(5)) == true) ? 1 : 0;
static const int greater_u64_max_five_n = ((operator>(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(5))) == true) ? 1 : 0;
static const int greater_u64_top_bit_vs_top_bit_minus_one_b = ((uint64_t(0x8000000000000000ull) > uint64_t(0x7FFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int greater_u64_top_bit_vs_top_bit_minus_one_n = ((operator>(uint64_t(0x8000000000000000ull), uint64_t(0x7FFFFFFFFFFFFFFFull))) == true) ? 1 : 0;

void check_greater()
{
    switch (0) { case 0: case greater_i8_min_max_b: break; }
    switch (0) { case 0: case greater_i8_min_max_n: break; }
    switch (0) { case 0: case greater_i8_max_min_b: break; }
    switch (0) { case 0: case greater_i8_max_min_n: break; }
    switch (0) { case 0: case greater_i8_five_five_b: break; }
    switch (0) { case 0: case greater_i8_five_five_n: break; }
    switch (0) { case 0: case greater_i8_zero_max_b: break; }
    switch (0) { case 0: case greater_i8_zero_max_n: break; }
    switch (0) { case 0: case greater_i8_max_five_b: break; }
    switch (0) { case 0: case greater_i8_max_five_n: break; }
    switch (0) { case 0: case greater_i8_minus_one_zero_b: break; }
    switch (0) { case 0: case greater_i8_minus_one_zero_n: break; }
    switch (0) { case 0: case greater_i8_zero_minus_one_b: break; }
    switch (0) { case 0: case greater_i8_zero_minus_one_n: break; }
    switch (0) { case 0: case greater_u8_min_max_b: break; }
    switch (0) { case 0: case greater_u8_min_max_n: break; }
    switch (0) { case 0: case greater_u8_max_min_b: break; }
    switch (0) { case 0: case greater_u8_max_min_n: break; }
    switch (0) { case 0: case greater_u8_five_five_b: break; }
    switch (0) { case 0: case greater_u8_five_five_n: break; }
    switch (0) { case 0: case greater_u8_max_five_b: break; }
    switch (0) { case 0: case greater_u8_max_five_n: break; }
    switch (0) { case 0: case greater_u8_top_bit_vs_top_bit_minus_one_b: break; }
    switch (0) { case 0: case greater_u8_top_bit_vs_top_bit_minus_one_n: break; }
    switch (0) { case 0: case greater_i64_min_max_b: break; }
    switch (0) { case 0: case greater_i64_min_max_n: break; }
    switch (0) { case 0: case greater_i64_max_min_b: break; }
    switch (0) { case 0: case greater_i64_max_min_n: break; }
    switch (0) { case 0: case greater_i64_five_five_b: break; }
    switch (0) { case 0: case greater_i64_five_five_n: break; }
    switch (0) { case 0: case greater_i64_zero_max_b: break; }
    switch (0) { case 0: case greater_i64_zero_max_n: break; }
    switch (0) { case 0: case greater_i64_max_five_b: break; }
    switch (0) { case 0: case greater_i64_max_five_n: break; }
    switch (0) { case 0: case greater_i64_minus_one_zero_b: break; }
    switch (0) { case 0: case greater_i64_minus_one_zero_n: break; }
    switch (0) { case 0: case greater_i64_zero_minus_one_b: break; }
    switch (0) { case 0: case greater_i64_zero_minus_one_n: break; }
    switch (0) { case 0: case greater_u64_min_max_b: break; }
    switch (0) { case 0: case greater_u64_min_max_n: break; }
    switch (0) { case 0: case greater_u64_max_min_b: break; }
    switch (0) { case 0: case greater_u64_max_min_n: break; }
    switch (0) { case 0: case greater_u64_five_five_b: break; }
    switch (0) { case 0: case greater_u64_five_five_n: break; }
    switch (0) { case 0: case greater_u64_max_five_b: break; }
    switch (0) { case 0: case greater_u64_max_five_n: break; }
    switch (0) { case 0: case greater_u64_top_bit_vs_top_bit_minus_one_b: break; }
    switch (0) { case 0: case greater_u64_top_bit_vs_top_bit_minus_one_n: break; }
}

// ------------------------------------------------------------------------------------------------
// leq `<=`
// ------------------------------------------------------------------------------------------------

static const int leq_i8_min_max_b = ((int8_t(-128) <= int8_t(127)) == true) ? 1 : 0;
static const int leq_i8_min_max_n = ((operator<=(int8_t(-128), int8_t(127))) == true) ? 1 : 0;
static const int leq_i8_max_min_b = ((int8_t(127) <= int8_t(-128)) == false) ? 1 : 0;
static const int leq_i8_max_min_n = ((operator<=(int8_t(127), int8_t(-128))) == false) ? 1 : 0;
static const int leq_i8_five_five_b = ((int8_t(5) <= int8_t(5)) == true) ? 1 : 0;
static const int leq_i8_five_five_n = ((operator<=(int8_t(5), int8_t(5))) == true) ? 1 : 0;
static const int leq_i8_zero_max_b = ((int8_t(0) <= int8_t(127)) == true) ? 1 : 0;
static const int leq_i8_zero_max_n = ((operator<=(int8_t(0), int8_t(127))) == true) ? 1 : 0;
static const int leq_i8_max_five_b = ((int8_t(127) <= int8_t(5)) == false) ? 1 : 0;
static const int leq_i8_max_five_n = ((operator<=(int8_t(127), int8_t(5))) == false) ? 1 : 0;
static const int leq_i8_minus_one_zero_b = ((int8_t(-1) <= int8_t(0)) == true) ? 1 : 0;
static const int leq_i8_minus_one_zero_n = ((operator<=(int8_t(-1), int8_t(0))) == true) ? 1 : 0;
static const int leq_i8_zero_minus_one_b = ((int8_t(0) <= int8_t(-1)) == false) ? 1 : 0;
static const int leq_i8_zero_minus_one_n = ((operator<=(int8_t(0), int8_t(-1))) == false) ? 1 : 0;
static const int leq_u8_min_max_b = ((uint8_t(0x00) <= uint8_t(0xFF)) == true) ? 1 : 0;
static const int leq_u8_min_max_n = ((operator<=(uint8_t(0x00), uint8_t(0xFF))) == true) ? 1 : 0;
static const int leq_u8_max_min_b = ((uint8_t(0xFF) <= uint8_t(0x00)) == false) ? 1 : 0;
static const int leq_u8_max_min_n = ((operator<=(uint8_t(0xFF), uint8_t(0x00))) == false) ? 1 : 0;
static const int leq_u8_five_five_b = ((uint8_t(0x05) <= uint8_t(0x05)) == true) ? 1 : 0;
static const int leq_u8_five_five_n = ((operator<=(uint8_t(0x05), uint8_t(0x05))) == true) ? 1 : 0;
static const int leq_u8_max_five_b = ((uint8_t(0xFF) <= uint8_t(0x05)) == false) ? 1 : 0;
static const int leq_u8_max_five_n = ((operator<=(uint8_t(0xFF), uint8_t(0x05))) == false) ? 1 : 0;
static const int leq_u8_top_bit_vs_top_bit_minus_one_b = ((uint8_t(0x80) <= uint8_t(0x7F)) == false) ? 1 : 0;
static const int leq_u8_top_bit_vs_top_bit_minus_one_n = ((operator<=(uint8_t(0x80), uint8_t(0x7F))) == false) ? 1 : 0;
static const int leq_i64_min_max_b = ((int64_t(0x8000000000000000ull) <= int64_t(0x7FFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int leq_i64_min_max_n = ((operator<=(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == true) ? 1 : 0;
static const int leq_i64_max_min_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) <= int64_t(0x8000000000000000ull)) == false) ? 1 : 0;
static const int leq_i64_max_min_n = ((operator<=(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0x8000000000000000ull))) == false) ? 1 : 0;
static const int leq_i64_five_five_b = ((int64_t(5) <= int64_t(5)) == true) ? 1 : 0;
static const int leq_i64_five_five_n = ((operator<=(int64_t(5), int64_t(5))) == true) ? 1 : 0;
static const int leq_i64_zero_max_b = ((int64_t(0) <= int64_t(0x7FFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int leq_i64_zero_max_n = ((operator<=(int64_t(0), int64_t(0x7FFFFFFFFFFFFFFFull))) == true) ? 1 : 0;
static const int leq_i64_max_five_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) <= int64_t(5)) == false) ? 1 : 0;
static const int leq_i64_max_five_n = ((operator<=(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(5))) == false) ? 1 : 0;
static const int leq_i64_minus_one_zero_b = ((int64_t(-1) <= int64_t(0)) == true) ? 1 : 0;
static const int leq_i64_minus_one_zero_n = ((operator<=(int64_t(-1), int64_t(0))) == true) ? 1 : 0;
static const int leq_i64_zero_minus_one_b = ((int64_t(0) <= int64_t(-1)) == false) ? 1 : 0;
static const int leq_i64_zero_minus_one_n = ((operator<=(int64_t(0), int64_t(-1))) == false) ? 1 : 0;
static const int leq_u64_min_max_b = ((uint64_t(0) <= uint64_t(0xFFFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int leq_u64_min_max_n = ((operator<=(uint64_t(0), uint64_t(0xFFFFFFFFFFFFFFFFull))) == true) ? 1 : 0;
static const int leq_u64_max_min_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) <= uint64_t(0)) == false) ? 1 : 0;
static const int leq_u64_max_min_n = ((operator<=(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0))) == false) ? 1 : 0;
static const int leq_u64_five_five_b = ((uint64_t(5) <= uint64_t(5)) == true) ? 1 : 0;
static const int leq_u64_five_five_n = ((operator<=(uint64_t(5), uint64_t(5))) == true) ? 1 : 0;
static const int leq_u64_max_five_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) <= uint64_t(5)) == false) ? 1 : 0;
static const int leq_u64_max_five_n = ((operator<=(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(5))) == false) ? 1 : 0;
static const int leq_u64_top_bit_vs_top_bit_minus_one_b = ((uint64_t(0x8000000000000000ull) <= uint64_t(0x7FFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int leq_u64_top_bit_vs_top_bit_minus_one_n = ((operator<=(uint64_t(0x8000000000000000ull), uint64_t(0x7FFFFFFFFFFFFFFFull))) == false) ? 1 : 0;

void check_leq()
{
    switch (0) { case 0: case leq_i8_min_max_b: break; }
    switch (0) { case 0: case leq_i8_min_max_n: break; }
    switch (0) { case 0: case leq_i8_max_min_b: break; }
    switch (0) { case 0: case leq_i8_max_min_n: break; }
    switch (0) { case 0: case leq_i8_five_five_b: break; }
    switch (0) { case 0: case leq_i8_five_five_n: break; }
    switch (0) { case 0: case leq_i8_zero_max_b: break; }
    switch (0) { case 0: case leq_i8_zero_max_n: break; }
    switch (0) { case 0: case leq_i8_max_five_b: break; }
    switch (0) { case 0: case leq_i8_max_five_n: break; }
    switch (0) { case 0: case leq_i8_minus_one_zero_b: break; }
    switch (0) { case 0: case leq_i8_minus_one_zero_n: break; }
    switch (0) { case 0: case leq_i8_zero_minus_one_b: break; }
    switch (0) { case 0: case leq_i8_zero_minus_one_n: break; }
    switch (0) { case 0: case leq_u8_min_max_b: break; }
    switch (0) { case 0: case leq_u8_min_max_n: break; }
    switch (0) { case 0: case leq_u8_max_min_b: break; }
    switch (0) { case 0: case leq_u8_max_min_n: break; }
    switch (0) { case 0: case leq_u8_five_five_b: break; }
    switch (0) { case 0: case leq_u8_five_five_n: break; }
    switch (0) { case 0: case leq_u8_max_five_b: break; }
    switch (0) { case 0: case leq_u8_max_five_n: break; }
    switch (0) { case 0: case leq_u8_top_bit_vs_top_bit_minus_one_b: break; }
    switch (0) { case 0: case leq_u8_top_bit_vs_top_bit_minus_one_n: break; }
    switch (0) { case 0: case leq_i64_min_max_b: break; }
    switch (0) { case 0: case leq_i64_min_max_n: break; }
    switch (0) { case 0: case leq_i64_max_min_b: break; }
    switch (0) { case 0: case leq_i64_max_min_n: break; }
    switch (0) { case 0: case leq_i64_five_five_b: break; }
    switch (0) { case 0: case leq_i64_five_five_n: break; }
    switch (0) { case 0: case leq_i64_zero_max_b: break; }
    switch (0) { case 0: case leq_i64_zero_max_n: break; }
    switch (0) { case 0: case leq_i64_max_five_b: break; }
    switch (0) { case 0: case leq_i64_max_five_n: break; }
    switch (0) { case 0: case leq_i64_minus_one_zero_b: break; }
    switch (0) { case 0: case leq_i64_minus_one_zero_n: break; }
    switch (0) { case 0: case leq_i64_zero_minus_one_b: break; }
    switch (0) { case 0: case leq_i64_zero_minus_one_n: break; }
    switch (0) { case 0: case leq_u64_min_max_b: break; }
    switch (0) { case 0: case leq_u64_min_max_n: break; }
    switch (0) { case 0: case leq_u64_max_min_b: break; }
    switch (0) { case 0: case leq_u64_max_min_n: break; }
    switch (0) { case 0: case leq_u64_five_five_b: break; }
    switch (0) { case 0: case leq_u64_five_five_n: break; }
    switch (0) { case 0: case leq_u64_max_five_b: break; }
    switch (0) { case 0: case leq_u64_max_five_n: break; }
    switch (0) { case 0: case leq_u64_top_bit_vs_top_bit_minus_one_b: break; }
    switch (0) { case 0: case leq_u64_top_bit_vs_top_bit_minus_one_n: break; }
}

// ------------------------------------------------------------------------------------------------
// geq `>=`
// ------------------------------------------------------------------------------------------------

static const int geq_i8_min_max_b = ((int8_t(-128) >= int8_t(127)) == false) ? 1 : 0;
static const int geq_i8_min_max_n = ((operator>=(int8_t(-128), int8_t(127))) == false) ? 1 : 0;
static const int geq_i8_max_min_b = ((int8_t(127) >= int8_t(-128)) == true) ? 1 : 0;
static const int geq_i8_max_min_n = ((operator>=(int8_t(127), int8_t(-128))) == true) ? 1 : 0;
static const int geq_i8_five_five_b = ((int8_t(5) >= int8_t(5)) == true) ? 1 : 0;
static const int geq_i8_five_five_n = ((operator>=(int8_t(5), int8_t(5))) == true) ? 1 : 0;
static const int geq_i8_zero_max_b = ((int8_t(0) >= int8_t(127)) == false) ? 1 : 0;
static const int geq_i8_zero_max_n = ((operator>=(int8_t(0), int8_t(127))) == false) ? 1 : 0;
static const int geq_i8_max_five_b = ((int8_t(127) >= int8_t(5)) == true) ? 1 : 0;
static const int geq_i8_max_five_n = ((operator>=(int8_t(127), int8_t(5))) == true) ? 1 : 0;
static const int geq_i8_minus_one_zero_b = ((int8_t(-1) >= int8_t(0)) == false) ? 1 : 0;
static const int geq_i8_minus_one_zero_n = ((operator>=(int8_t(-1), int8_t(0))) == false) ? 1 : 0;
static const int geq_i8_zero_minus_one_b = ((int8_t(0) >= int8_t(-1)) == true) ? 1 : 0;
static const int geq_i8_zero_minus_one_n = ((operator>=(int8_t(0), int8_t(-1))) == true) ? 1 : 0;
static const int geq_u8_min_max_b = ((uint8_t(0x00) >= uint8_t(0xFF)) == false) ? 1 : 0;
static const int geq_u8_min_max_n = ((operator>=(uint8_t(0x00), uint8_t(0xFF))) == false) ? 1 : 0;
static const int geq_u8_max_min_b = ((uint8_t(0xFF) >= uint8_t(0x00)) == true) ? 1 : 0;
static const int geq_u8_max_min_n = ((operator>=(uint8_t(0xFF), uint8_t(0x00))) == true) ? 1 : 0;
static const int geq_u8_five_five_b = ((uint8_t(0x05) >= uint8_t(0x05)) == true) ? 1 : 0;
static const int geq_u8_five_five_n = ((operator>=(uint8_t(0x05), uint8_t(0x05))) == true) ? 1 : 0;
static const int geq_u8_max_five_b = ((uint8_t(0xFF) >= uint8_t(0x05)) == true) ? 1 : 0;
static const int geq_u8_max_five_n = ((operator>=(uint8_t(0xFF), uint8_t(0x05))) == true) ? 1 : 0;
static const int geq_u8_top_bit_vs_top_bit_minus_one_b = ((uint8_t(0x80) >= uint8_t(0x7F)) == true) ? 1 : 0;
static const int geq_u8_top_bit_vs_top_bit_minus_one_n = ((operator>=(uint8_t(0x80), uint8_t(0x7F))) == true) ? 1 : 0;
static const int geq_i64_min_max_b = ((int64_t(0x8000000000000000ull) >= int64_t(0x7FFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int geq_i64_min_max_n = ((operator>=(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == false) ? 1 : 0;
static const int geq_i64_max_min_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) >= int64_t(0x8000000000000000ull)) == true) ? 1 : 0;
static const int geq_i64_max_min_n = ((operator>=(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0x8000000000000000ull))) == true) ? 1 : 0;
static const int geq_i64_five_five_b = ((int64_t(5) >= int64_t(5)) == true) ? 1 : 0;
static const int geq_i64_five_five_n = ((operator>=(int64_t(5), int64_t(5))) == true) ? 1 : 0;
static const int geq_i64_zero_max_b = ((int64_t(0) >= int64_t(0x7FFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int geq_i64_zero_max_n = ((operator>=(int64_t(0), int64_t(0x7FFFFFFFFFFFFFFFull))) == false) ? 1 : 0;
static const int geq_i64_max_five_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) >= int64_t(5)) == true) ? 1 : 0;
static const int geq_i64_max_five_n = ((operator>=(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(5))) == true) ? 1 : 0;
static const int geq_i64_minus_one_zero_b = ((int64_t(-1) >= int64_t(0)) == false) ? 1 : 0;
static const int geq_i64_minus_one_zero_n = ((operator>=(int64_t(-1), int64_t(0))) == false) ? 1 : 0;
static const int geq_i64_zero_minus_one_b = ((int64_t(0) >= int64_t(-1)) == true) ? 1 : 0;
static const int geq_i64_zero_minus_one_n = ((operator>=(int64_t(0), int64_t(-1))) == true) ? 1 : 0;
static const int geq_u64_min_max_b = ((uint64_t(0) >= uint64_t(0xFFFFFFFFFFFFFFFFull)) == false) ? 1 : 0;
static const int geq_u64_min_max_n = ((operator>=(uint64_t(0), uint64_t(0xFFFFFFFFFFFFFFFFull))) == false) ? 1 : 0;
static const int geq_u64_max_min_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) >= uint64_t(0)) == true) ? 1 : 0;
static const int geq_u64_max_min_n = ((operator>=(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0))) == true) ? 1 : 0;
static const int geq_u64_five_five_b = ((uint64_t(5) >= uint64_t(5)) == true) ? 1 : 0;
static const int geq_u64_five_five_n = ((operator>=(uint64_t(5), uint64_t(5))) == true) ? 1 : 0;
static const int geq_u64_max_five_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) >= uint64_t(5)) == true) ? 1 : 0;
static const int geq_u64_max_five_n = ((operator>=(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(5))) == true) ? 1 : 0;
static const int geq_u64_top_bit_vs_top_bit_minus_one_b = ((uint64_t(0x8000000000000000ull) >= uint64_t(0x7FFFFFFFFFFFFFFFull)) == true) ? 1 : 0;
static const int geq_u64_top_bit_vs_top_bit_minus_one_n = ((operator>=(uint64_t(0x8000000000000000ull), uint64_t(0x7FFFFFFFFFFFFFFFull))) == true) ? 1 : 0;

void check_geq()
{
    switch (0) { case 0: case geq_i8_min_max_b: break; }
    switch (0) { case 0: case geq_i8_min_max_n: break; }
    switch (0) { case 0: case geq_i8_max_min_b: break; }
    switch (0) { case 0: case geq_i8_max_min_n: break; }
    switch (0) { case 0: case geq_i8_five_five_b: break; }
    switch (0) { case 0: case geq_i8_five_five_n: break; }
    switch (0) { case 0: case geq_i8_zero_max_b: break; }
    switch (0) { case 0: case geq_i8_zero_max_n: break; }
    switch (0) { case 0: case geq_i8_max_five_b: break; }
    switch (0) { case 0: case geq_i8_max_five_n: break; }
    switch (0) { case 0: case geq_i8_minus_one_zero_b: break; }
    switch (0) { case 0: case geq_i8_minus_one_zero_n: break; }
    switch (0) { case 0: case geq_i8_zero_minus_one_b: break; }
    switch (0) { case 0: case geq_i8_zero_minus_one_n: break; }
    switch (0) { case 0: case geq_u8_min_max_b: break; }
    switch (0) { case 0: case geq_u8_min_max_n: break; }
    switch (0) { case 0: case geq_u8_max_min_b: break; }
    switch (0) { case 0: case geq_u8_max_min_n: break; }
    switch (0) { case 0: case geq_u8_five_five_b: break; }
    switch (0) { case 0: case geq_u8_five_five_n: break; }
    switch (0) { case 0: case geq_u8_max_five_b: break; }
    switch (0) { case 0: case geq_u8_max_five_n: break; }
    switch (0) { case 0: case geq_u8_top_bit_vs_top_bit_minus_one_b: break; }
    switch (0) { case 0: case geq_u8_top_bit_vs_top_bit_minus_one_n: break; }
    switch (0) { case 0: case geq_i64_min_max_b: break; }
    switch (0) { case 0: case geq_i64_min_max_n: break; }
    switch (0) { case 0: case geq_i64_max_min_b: break; }
    switch (0) { case 0: case geq_i64_max_min_n: break; }
    switch (0) { case 0: case geq_i64_five_five_b: break; }
    switch (0) { case 0: case geq_i64_five_five_n: break; }
    switch (0) { case 0: case geq_i64_zero_max_b: break; }
    switch (0) { case 0: case geq_i64_zero_max_n: break; }
    switch (0) { case 0: case geq_i64_max_five_b: break; }
    switch (0) { case 0: case geq_i64_max_five_n: break; }
    switch (0) { case 0: case geq_i64_minus_one_zero_b: break; }
    switch (0) { case 0: case geq_i64_minus_one_zero_n: break; }
    switch (0) { case 0: case geq_i64_zero_minus_one_b: break; }
    switch (0) { case 0: case geq_i64_zero_minus_one_n: break; }
    switch (0) { case 0: case geq_u64_min_max_b: break; }
    switch (0) { case 0: case geq_u64_min_max_n: break; }
    switch (0) { case 0: case geq_u64_max_min_b: break; }
    switch (0) { case 0: case geq_u64_max_min_n: break; }
    switch (0) { case 0: case geq_u64_five_five_b: break; }
    switch (0) { case 0: case geq_u64_five_five_n: break; }
    switch (0) { case 0: case geq_u64_max_five_b: break; }
    switch (0) { case 0: case geq_u64_max_five_n: break; }
    switch (0) { case 0: case geq_u64_top_bit_vs_top_bit_minus_one_b: break; }
    switch (0) { case 0: case geq_u64_top_bit_vs_top_bit_minus_one_n: break; }
}

// ------------------------------------------------------------------------------------------------
// not `!`
// The built-in path takes `!` only for `bool` operands.
// ------------------------------------------------------------------------------------------------

static const int not_bool_false_b = ((!false) == true) ? 1 : 0;
static const int not_bool_false_n = ((operator!(false)) == true) ? 1 : 0;
static const int not_bool_true_b = ((!true) == false) ? 1 : 0;
static const int not_bool_true_n = ((operator!(true)) == false) ? 1 : 0;

void check_not()
{
    switch (0) { case 0: case not_bool_false_b: break; }
    switch (0) { case 0: case not_bool_false_n: break; }
    switch (0) { case 0: case not_bool_true_b: break; }
    switch (0) { case 0: case not_bool_true_n: break; }
}

// ------------------------------------------------------------------------------------------------
// and `&&`
// `&&` is never rewritten to a `BuiltinOperatorExpr`, so both spellings take
// the named path: `_infix` is `a && b` and `_call` is `operator&&(a, b)`.
// ------------------------------------------------------------------------------------------------

static const int and_bool_false_false_infix = ((false && false) == false) ? 1 : 0;
static const int and_bool_false_false_call = ((operator&&(false, false)) == false) ? 1 : 0;
static const int and_bool_false_true_infix = ((false && true) == false) ? 1 : 0;
static const int and_bool_false_true_call = ((operator&&(false, true)) == false) ? 1 : 0;
static const int and_bool_true_false_infix = ((true && false) == false) ? 1 : 0;
static const int and_bool_true_false_call = ((operator&&(true, false)) == false) ? 1 : 0;
static const int and_bool_true_true_infix = ((true && true) == true) ? 1 : 0;
static const int and_bool_true_true_call = ((operator&&(true, true)) == true) ? 1 : 0;

void check_and()
{
    switch (0) { case 0: case and_bool_false_false_infix: break; }
    switch (0) { case 0: case and_bool_false_false_call: break; }
    switch (0) { case 0: case and_bool_false_true_infix: break; }
    switch (0) { case 0: case and_bool_false_true_call: break; }
    switch (0) { case 0: case and_bool_true_false_infix: break; }
    switch (0) { case 0: case and_bool_true_false_call: break; }
    switch (0) { case 0: case and_bool_true_true_infix: break; }
    switch (0) { case 0: case and_bool_true_true_call: break; }
}

// ------------------------------------------------------------------------------------------------
// or `||`
// `||` is never rewritten to a `BuiltinOperatorExpr`, so both spellings take
// the named path: `_infix` is `a || b` and `_call` is `operator||(a, b)`.
// ------------------------------------------------------------------------------------------------

static const int or_bool_false_false_infix = ((false || false) == false) ? 1 : 0;
static const int or_bool_false_false_call = ((operator||(false, false)) == false) ? 1 : 0;
static const int or_bool_false_true_infix = ((false || true) == true) ? 1 : 0;
static const int or_bool_false_true_call = ((operator||(false, true)) == true) ? 1 : 0;
static const int or_bool_true_false_infix = ((true || false) == true) ? 1 : 0;
static const int or_bool_true_false_call = ((operator||(true, false)) == true) ? 1 : 0;
static const int or_bool_true_true_infix = ((true || true) == true) ? 1 : 0;
static const int or_bool_true_true_call = ((operator||(true, true)) == true) ? 1 : 0;

void check_or()
{
    switch (0) { case 0: case or_bool_false_false_infix: break; }
    switch (0) { case 0: case or_bool_false_false_call: break; }
    switch (0) { case 0: case or_bool_false_true_infix: break; }
    switch (0) { case 0: case or_bool_false_true_call: break; }
    switch (0) { case 0: case or_bool_true_false_infix: break; }
    switch (0) { case 0: case or_bool_true_false_call: break; }
    switch (0) { case 0: case or_bool_true_true_infix: break; }
    switch (0) { case 0: case or_bool_true_true_call: break; }
}

// ------------------------------------------------------------------------------------------------
// cond `?:`
// `?:` is never rewritten to a `BuiltinOperatorExpr` and has no `operator` call
// spelling, so it is folded only through the named path. The result keeps the
// operand type.
// ------------------------------------------------------------------------------------------------

static const int cond_i8_true_picks_min = ((true ? int8_t(-128) : int8_t(127)) == int8_t(-128)) ? 1 : 0;
static const int cond_i8_false_picks_max = ((false ? int8_t(-128) : int8_t(127)) == int8_t(127)) ? 1 : 0;
static const int cond_u8_true_picks_min = ((true ? uint8_t(0x00) : uint8_t(0xFF)) == uint8_t(0x00)) ? 1 : 0;
static const int cond_u8_false_picks_max = ((false ? uint8_t(0x00) : uint8_t(0xFF)) == uint8_t(0xFF)) ? 1 : 0;
static const int cond_i64_true_picks_min = ((true ? int64_t(0x8000000000000000ull) : int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int cond_i64_false_picks_max = ((false ? int64_t(0x8000000000000000ull) : int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int cond_u64_true_picks_min = ((true ? uint64_t(0) : uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0)) ? 1 : 0;
static const int cond_u64_false_picks_max = ((false ? uint64_t(0) : uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;

void check_cond()
{
    switch (0) { case 0: case cond_i8_true_picks_min: break; }
    switch (0) { case 0: case cond_i8_false_picks_max: break; }
    switch (0) { case 0: case cond_u8_true_picks_min: break; }
    switch (0) { case 0: case cond_u8_false_picks_max: break; }
    switch (0) { case 0: case cond_i64_true_picks_min: break; }
    switch (0) { case 0: case cond_i64_false_picks_max: break; }
    switch (0) { case 0: case cond_u64_true_picks_min: break; }
    switch (0) { case 0: case cond_u64_false_picks_max: break; }
}
