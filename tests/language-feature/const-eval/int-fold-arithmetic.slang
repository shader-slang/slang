//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

// Front-end constant folding of the integer arithmetic operators `+ - * / %`, unary `-`
// and unary `+` on int8_t, uint8_t, int64_t and uint64_t operands.
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
// add `+`
// ------------------------------------------------------------------------------------------------

static const int add_i8_max_plus_one_b = ((int8_t(127) + int8_t(1)) == int8_t(-128)) ? 1 : 0;
static const int add_i8_max_plus_one_n = ((operator+(int8_t(127), int8_t(1))) == int8_t(-128)) ? 1 : 0;
static const int add_i8_zero_plus_zero_b = ((int8_t(0) + int8_t(0)) == int8_t(0)) ? 1 : 0;
static const int add_i8_zero_plus_zero_n = ((operator+(int8_t(0), int8_t(0))) == int8_t(0)) ? 1 : 0;
static const int add_i8_five_plus_seven_b = ((int8_t(5) + int8_t(7)) == int8_t(12)) ? 1 : 0;
static const int add_i8_five_plus_seven_n = ((operator+(int8_t(5), int8_t(7))) == int8_t(12)) ? 1 : 0;
static const int add_i8_min_plus_minus_one_b = ((int8_t(-128) + int8_t(-1)) == int8_t(127)) ? 1 : 0;
static const int add_i8_min_plus_minus_one_n = ((operator+(int8_t(-128), int8_t(-1))) == int8_t(127)) ? 1 : 0;
static const int add_i8_minus_one_plus_one_b = ((int8_t(-1) + int8_t(1)) == int8_t(0)) ? 1 : 0;
static const int add_i8_minus_one_plus_one_n = ((operator+(int8_t(-1), int8_t(1))) == int8_t(0)) ? 1 : 0;
static const int add_i8_min_plus_max_b = ((int8_t(-128) + int8_t(127)) == int8_t(-1)) ? 1 : 0;
static const int add_i8_min_plus_max_n = ((operator+(int8_t(-128), int8_t(127))) == int8_t(-1)) ? 1 : 0;
static const int add_u8_max_plus_one_b = ((uint8_t(0xFF) + uint8_t(0x01)) == uint8_t(0x00)) ? 1 : 0;
static const int add_u8_max_plus_one_n = ((operator+(uint8_t(0xFF), uint8_t(0x01))) == uint8_t(0x00)) ? 1 : 0;
static const int add_u8_zero_plus_zero_b = ((uint8_t(0x00) + uint8_t(0x00)) == uint8_t(0x00)) ? 1 : 0;
static const int add_u8_zero_plus_zero_n = ((operator+(uint8_t(0x00), uint8_t(0x00))) == uint8_t(0x00)) ? 1 : 0;
static const int add_u8_five_plus_seven_b = ((uint8_t(0x05) + uint8_t(0x07)) == uint8_t(0x0C)) ? 1 : 0;
static const int add_u8_five_plus_seven_n = ((operator+(uint8_t(0x05), uint8_t(0x07))) == uint8_t(0x0C)) ? 1 : 0;
static const int add_u8_max_plus_max_b = ((uint8_t(0xFF) + uint8_t(0xFF)) == uint8_t(0xFE)) ? 1 : 0;
static const int add_u8_max_plus_max_n = ((operator+(uint8_t(0xFF), uint8_t(0xFF))) == uint8_t(0xFE)) ? 1 : 0;
static const int add_i64_max_plus_one_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) + int64_t(1)) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int add_i64_max_plus_one_n = ((operator+(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(1))) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int add_i64_zero_plus_zero_b = ((int64_t(0) + int64_t(0)) == int64_t(0)) ? 1 : 0;
static const int add_i64_zero_plus_zero_n = ((operator+(int64_t(0), int64_t(0))) == int64_t(0)) ? 1 : 0;
static const int add_i64_five_plus_seven_b = ((int64_t(5) + int64_t(7)) == int64_t(12)) ? 1 : 0;
static const int add_i64_five_plus_seven_n = ((operator+(int64_t(5), int64_t(7))) == int64_t(12)) ? 1 : 0;
static const int add_i64_min_plus_minus_one_b = ((int64_t(0x8000000000000000ull) + int64_t(-1)) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int add_i64_min_plus_minus_one_n = ((operator+(int64_t(0x8000000000000000ull), int64_t(-1))) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int add_i64_minus_one_plus_one_b = ((int64_t(-1) + int64_t(1)) == int64_t(0)) ? 1 : 0;
static const int add_i64_minus_one_plus_one_n = ((operator+(int64_t(-1), int64_t(1))) == int64_t(0)) ? 1 : 0;
static const int add_i64_min_plus_max_b = ((int64_t(0x8000000000000000ull) + int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(-1)) ? 1 : 0;
static const int add_i64_min_plus_max_n = ((operator+(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(-1)) ? 1 : 0;
static const int add_u64_max_plus_one_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) + uint64_t(1)) == uint64_t(0)) ? 1 : 0;
static const int add_u64_max_plus_one_n = ((operator+(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(1))) == uint64_t(0)) ? 1 : 0;
static const int add_u64_zero_plus_zero_b = ((uint64_t(0) + uint64_t(0)) == uint64_t(0)) ? 1 : 0;
static const int add_u64_zero_plus_zero_n = ((operator+(uint64_t(0), uint64_t(0))) == uint64_t(0)) ? 1 : 0;
static const int add_u64_five_plus_seven_b = ((uint64_t(5) + uint64_t(7)) == uint64_t(12)) ? 1 : 0;
static const int add_u64_five_plus_seven_n = ((operator+(uint64_t(5), uint64_t(7))) == uint64_t(12)) ? 1 : 0;
static const int add_u64_max_plus_max_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) + uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0xFFFFFFFFFFFFFFFEull)) ? 1 : 0;
static const int add_u64_max_plus_max_n = ((operator+(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(0xFFFFFFFFFFFFFFFEull)) ? 1 : 0;

void check_add()
{
    switch (0) { case 0: case add_i8_max_plus_one_b: break; }
    switch (0) { case 0: case add_i8_max_plus_one_n: break; }
    switch (0) { case 0: case add_i8_zero_plus_zero_b: break; }
    switch (0) { case 0: case add_i8_zero_plus_zero_n: break; }
    switch (0) { case 0: case add_i8_five_plus_seven_b: break; }
    switch (0) { case 0: case add_i8_five_plus_seven_n: break; }
    switch (0) { case 0: case add_i8_min_plus_minus_one_b: break; }
    switch (0) { case 0: case add_i8_min_plus_minus_one_n: break; }
    switch (0) { case 0: case add_i8_minus_one_plus_one_b: break; }
    switch (0) { case 0: case add_i8_minus_one_plus_one_n: break; }
    switch (0) { case 0: case add_i8_min_plus_max_b: break; }
    switch (0) { case 0: case add_i8_min_plus_max_n: break; }
    switch (0) { case 0: case add_u8_max_plus_one_b: break; }
    switch (0) { case 0: case add_u8_max_plus_one_n: break; }
    switch (0) { case 0: case add_u8_zero_plus_zero_b: break; }
    switch (0) { case 0: case add_u8_zero_plus_zero_n: break; }
    switch (0) { case 0: case add_u8_five_plus_seven_b: break; }
    switch (0) { case 0: case add_u8_five_plus_seven_n: break; }
    switch (0) { case 0: case add_u8_max_plus_max_b: break; }
    switch (0) { case 0: case add_u8_max_plus_max_n: break; }
    switch (0) { case 0: case add_i64_max_plus_one_b: break; }
    switch (0) { case 0: case add_i64_max_plus_one_n: break; }
    switch (0) { case 0: case add_i64_zero_plus_zero_b: break; }
    switch (0) { case 0: case add_i64_zero_plus_zero_n: break; }
    switch (0) { case 0: case add_i64_five_plus_seven_b: break; }
    switch (0) { case 0: case add_i64_five_plus_seven_n: break; }
    switch (0) { case 0: case add_i64_min_plus_minus_one_b: break; }
    switch (0) { case 0: case add_i64_min_plus_minus_one_n: break; }
    switch (0) { case 0: case add_i64_minus_one_plus_one_b: break; }
    switch (0) { case 0: case add_i64_minus_one_plus_one_n: break; }
    switch (0) { case 0: case add_i64_min_plus_max_b: break; }
    switch (0) { case 0: case add_i64_min_plus_max_n: break; }
    switch (0) { case 0: case add_u64_max_plus_one_b: break; }
    switch (0) { case 0: case add_u64_max_plus_one_n: break; }
    switch (0) { case 0: case add_u64_zero_plus_zero_b: break; }
    switch (0) { case 0: case add_u64_zero_plus_zero_n: break; }
    switch (0) { case 0: case add_u64_five_plus_seven_b: break; }
    switch (0) { case 0: case add_u64_five_plus_seven_n: break; }
    switch (0) { case 0: case add_u64_max_plus_max_b: break; }
    switch (0) { case 0: case add_u64_max_plus_max_n: break; }
}

// ------------------------------------------------------------------------------------------------
// sub binary `-`
// ------------------------------------------------------------------------------------------------

static const int sub_i8_min_minus_one_b = ((int8_t(-128) - int8_t(1)) == int8_t(127)) ? 1 : 0;
static const int sub_i8_min_minus_one_n = ((operator-(int8_t(-128), int8_t(1))) == int8_t(127)) ? 1 : 0;
static const int sub_i8_seven_minus_five_b = ((int8_t(7) - int8_t(5)) == int8_t(2)) ? 1 : 0;
static const int sub_i8_seven_minus_five_n = ((operator-(int8_t(7), int8_t(5))) == int8_t(2)) ? 1 : 0;
static const int sub_i8_max_minus_min_b = ((int8_t(127) - int8_t(-128)) == int8_t(-1)) ? 1 : 0;
static const int sub_i8_max_minus_min_n = ((operator-(int8_t(127), int8_t(-128))) == int8_t(-1)) ? 1 : 0;
static const int sub_i8_min_minus_max_b = ((int8_t(-128) - int8_t(127)) == int8_t(1)) ? 1 : 0;
static const int sub_i8_min_minus_max_n = ((operator-(int8_t(-128), int8_t(127))) == int8_t(1)) ? 1 : 0;
static const int sub_i8_minus_one_minus_minus_one_b = ((int8_t(-1) - int8_t(-1)) == int8_t(0)) ? 1 : 0;
static const int sub_i8_minus_one_minus_minus_one_n = ((operator-(int8_t(-1), int8_t(-1))) == int8_t(0)) ? 1 : 0;
static const int sub_u8_min_minus_one_b = ((uint8_t(0x00) - uint8_t(0x01)) == uint8_t(0xFF)) ? 1 : 0;
static const int sub_u8_min_minus_one_n = ((operator-(uint8_t(0x00), uint8_t(0x01))) == uint8_t(0xFF)) ? 1 : 0;
static const int sub_u8_seven_minus_five_b = ((uint8_t(0x07) - uint8_t(0x05)) == uint8_t(0x02)) ? 1 : 0;
static const int sub_u8_seven_minus_five_n = ((operator-(uint8_t(0x07), uint8_t(0x05))) == uint8_t(0x02)) ? 1 : 0;
static const int sub_u8_one_minus_max_b = ((uint8_t(0x01) - uint8_t(0xFF)) == uint8_t(0x02)) ? 1 : 0;
static const int sub_u8_one_minus_max_n = ((operator-(uint8_t(0x01), uint8_t(0xFF))) == uint8_t(0x02)) ? 1 : 0;
static const int sub_u8_five_minus_seven_b = ((uint8_t(0x05) - uint8_t(0x07)) == uint8_t(0xFE)) ? 1 : 0;
static const int sub_u8_five_minus_seven_n = ((operator-(uint8_t(0x05), uint8_t(0x07))) == uint8_t(0xFE)) ? 1 : 0;
static const int sub_i64_min_minus_one_b = ((int64_t(0x8000000000000000ull) - int64_t(1)) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int sub_i64_min_minus_one_n = ((operator-(int64_t(0x8000000000000000ull), int64_t(1))) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int sub_i64_seven_minus_five_b = ((int64_t(7) - int64_t(5)) == int64_t(2)) ? 1 : 0;
static const int sub_i64_seven_minus_five_n = ((operator-(int64_t(7), int64_t(5))) == int64_t(2)) ? 1 : 0;
static const int sub_i64_max_minus_min_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) - int64_t(0x8000000000000000ull)) == int64_t(-1)) ? 1 : 0;
static const int sub_i64_max_minus_min_n = ((operator-(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0x8000000000000000ull))) == int64_t(-1)) ? 1 : 0;
static const int sub_i64_min_minus_max_b = ((int64_t(0x8000000000000000ull) - int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(1)) ? 1 : 0;
static const int sub_i64_min_minus_max_n = ((operator-(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(1)) ? 1 : 0;
static const int sub_i64_minus_one_minus_minus_one_b = ((int64_t(-1) - int64_t(-1)) == int64_t(0)) ? 1 : 0;
static const int sub_i64_minus_one_minus_minus_one_n = ((operator-(int64_t(-1), int64_t(-1))) == int64_t(0)) ? 1 : 0;
static const int sub_u64_min_minus_one_b = ((uint64_t(0) - uint64_t(1)) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int sub_u64_min_minus_one_n = ((operator-(uint64_t(0), uint64_t(1))) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int sub_u64_seven_minus_five_b = ((uint64_t(7) - uint64_t(5)) == uint64_t(2)) ? 1 : 0;
static const int sub_u64_seven_minus_five_n = ((operator-(uint64_t(7), uint64_t(5))) == uint64_t(2)) ? 1 : 0;
static const int sub_u64_one_minus_max_b = ((uint64_t(1) - uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(2)) ? 1 : 0;
static const int sub_u64_one_minus_max_n = ((operator-(uint64_t(1), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(2)) ? 1 : 0;
static const int sub_u64_five_minus_seven_b = ((uint64_t(5) - uint64_t(7)) == uint64_t(0xFFFFFFFFFFFFFFFEull)) ? 1 : 0;
static const int sub_u64_five_minus_seven_n = ((operator-(uint64_t(5), uint64_t(7))) == uint64_t(0xFFFFFFFFFFFFFFFEull)) ? 1 : 0;

void check_sub()
{
    switch (0) { case 0: case sub_i8_min_minus_one_b: break; }
    switch (0) { case 0: case sub_i8_min_minus_one_n: break; }
    switch (0) { case 0: case sub_i8_seven_minus_five_b: break; }
    switch (0) { case 0: case sub_i8_seven_minus_five_n: break; }
    switch (0) { case 0: case sub_i8_max_minus_min_b: break; }
    switch (0) { case 0: case sub_i8_max_minus_min_n: break; }
    switch (0) { case 0: case sub_i8_min_minus_max_b: break; }
    switch (0) { case 0: case sub_i8_min_minus_max_n: break; }
    switch (0) { case 0: case sub_i8_minus_one_minus_minus_one_b: break; }
    switch (0) { case 0: case sub_i8_minus_one_minus_minus_one_n: break; }
    switch (0) { case 0: case sub_u8_min_minus_one_b: break; }
    switch (0) { case 0: case sub_u8_min_minus_one_n: break; }
    switch (0) { case 0: case sub_u8_seven_minus_five_b: break; }
    switch (0) { case 0: case sub_u8_seven_minus_five_n: break; }
    switch (0) { case 0: case sub_u8_one_minus_max_b: break; }
    switch (0) { case 0: case sub_u8_one_minus_max_n: break; }
    switch (0) { case 0: case sub_u8_five_minus_seven_b: break; }
    switch (0) { case 0: case sub_u8_five_minus_seven_n: break; }
    switch (0) { case 0: case sub_i64_min_minus_one_b: break; }
    switch (0) { case 0: case sub_i64_min_minus_one_n: break; }
    switch (0) { case 0: case sub_i64_seven_minus_five_b: break; }
    switch (0) { case 0: case sub_i64_seven_minus_five_n: break; }
    switch (0) { case 0: case sub_i64_max_minus_min_b: break; }
    switch (0) { case 0: case sub_i64_max_minus_min_n: break; }
    switch (0) { case 0: case sub_i64_min_minus_max_b: break; }
    switch (0) { case 0: case sub_i64_min_minus_max_n: break; }
    switch (0) { case 0: case sub_i64_minus_one_minus_minus_one_b: break; }
    switch (0) { case 0: case sub_i64_minus_one_minus_minus_one_n: break; }
    switch (0) { case 0: case sub_u64_min_minus_one_b: break; }
    switch (0) { case 0: case sub_u64_min_minus_one_n: break; }
    switch (0) { case 0: case sub_u64_seven_minus_five_b: break; }
    switch (0) { case 0: case sub_u64_seven_minus_five_n: break; }
    switch (0) { case 0: case sub_u64_one_minus_max_b: break; }
    switch (0) { case 0: case sub_u64_one_minus_max_n: break; }
    switch (0) { case 0: case sub_u64_five_minus_seven_b: break; }
    switch (0) { case 0: case sub_u64_five_minus_seven_n: break; }
}

// ------------------------------------------------------------------------------------------------
// mul `*`
// ------------------------------------------------------------------------------------------------

static const int mul_i8_max_times_two_b = ((int8_t(127) * int8_t(2)) == int8_t(-2)) ? 1 : 0;
static const int mul_i8_max_times_two_n = ((operator*(int8_t(127), int8_t(2))) == int8_t(-2)) ? 1 : 0;
static const int mul_i8_half_width_bit_squared_b = ((int8_t(16) * int8_t(16)) == int8_t(0)) ? 1 : 0;
static const int mul_i8_half_width_bit_squared_n = ((operator*(int8_t(16), int8_t(16))) == int8_t(0)) ? 1 : 0;
static const int mul_i8_six_times_seven_b = ((int8_t(6) * int8_t(7)) == int8_t(42)) ? 1 : 0;
static const int mul_i8_six_times_seven_n = ((operator*(int8_t(6), int8_t(7))) == int8_t(42)) ? 1 : 0;
static const int mul_i8_max_times_zero_b = ((int8_t(127) * int8_t(0)) == int8_t(0)) ? 1 : 0;
static const int mul_i8_max_times_zero_n = ((operator*(int8_t(127), int8_t(0))) == int8_t(0)) ? 1 : 0;
static const int mul_i8_min_times_minus_one_b = ((int8_t(-128) * int8_t(-1)) == int8_t(-128)) ? 1 : 0;
static const int mul_i8_min_times_minus_one_n = ((operator*(int8_t(-128), int8_t(-1))) == int8_t(-128)) ? 1 : 0;
static const int mul_i8_minus_one_times_minus_one_b = ((int8_t(-1) * int8_t(-1)) == int8_t(1)) ? 1 : 0;
static const int mul_i8_minus_one_times_minus_one_n = ((operator*(int8_t(-1), int8_t(-1))) == int8_t(1)) ? 1 : 0;
static const int mul_i8_min_times_min_b = ((int8_t(-128) * int8_t(-128)) == int8_t(0)) ? 1 : 0;
static const int mul_i8_min_times_min_n = ((operator*(int8_t(-128), int8_t(-128))) == int8_t(0)) ? 1 : 0;
static const int mul_i8_max_times_minus_one_b = ((int8_t(127) * int8_t(-1)) == int8_t(-127)) ? 1 : 0;
static const int mul_i8_max_times_minus_one_n = ((operator*(int8_t(127), int8_t(-1))) == int8_t(-127)) ? 1 : 0;
static const int mul_u8_max_times_two_b = ((uint8_t(0xFF) * uint8_t(0x02)) == uint8_t(0xFE)) ? 1 : 0;
static const int mul_u8_max_times_two_n = ((operator*(uint8_t(0xFF), uint8_t(0x02))) == uint8_t(0xFE)) ? 1 : 0;
static const int mul_u8_half_width_bit_squared_b = ((uint8_t(0x10) * uint8_t(0x10)) == uint8_t(0x00)) ? 1 : 0;
static const int mul_u8_half_width_bit_squared_n = ((operator*(uint8_t(0x10), uint8_t(0x10))) == uint8_t(0x00)) ? 1 : 0;
static const int mul_u8_six_times_seven_b = ((uint8_t(0x06) * uint8_t(0x07)) == uint8_t(0x2A)) ? 1 : 0;
static const int mul_u8_six_times_seven_n = ((operator*(uint8_t(0x06), uint8_t(0x07))) == uint8_t(0x2A)) ? 1 : 0;
static const int mul_u8_max_times_zero_b = ((uint8_t(0xFF) * uint8_t(0x00)) == uint8_t(0x00)) ? 1 : 0;
static const int mul_u8_max_times_zero_n = ((operator*(uint8_t(0xFF), uint8_t(0x00))) == uint8_t(0x00)) ? 1 : 0;
static const int mul_u8_max_times_max_b = ((uint8_t(0xFF) * uint8_t(0xFF)) == uint8_t(0x01)) ? 1 : 0;
static const int mul_u8_max_times_max_n = ((operator*(uint8_t(0xFF), uint8_t(0xFF))) == uint8_t(0x01)) ? 1 : 0;
static const int mul_i64_max_times_two_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) * int64_t(2)) == int64_t(-2)) ? 1 : 0;
static const int mul_i64_max_times_two_n = ((operator*(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(2))) == int64_t(-2)) ? 1 : 0;
static const int mul_i64_half_width_bit_squared_b = ((int64_t(0x0000000100000000ull) * int64_t(0x0000000100000000ull)) == int64_t(0)) ? 1 : 0;
static const int mul_i64_half_width_bit_squared_n = ((operator*(int64_t(0x0000000100000000ull), int64_t(0x0000000100000000ull))) == int64_t(0)) ? 1 : 0;
static const int mul_i64_six_times_seven_b = ((int64_t(6) * int64_t(7)) == int64_t(42)) ? 1 : 0;
static const int mul_i64_six_times_seven_n = ((operator*(int64_t(6), int64_t(7))) == int64_t(42)) ? 1 : 0;
static const int mul_i64_max_times_zero_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) * int64_t(0)) == int64_t(0)) ? 1 : 0;
static const int mul_i64_max_times_zero_n = ((operator*(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0))) == int64_t(0)) ? 1 : 0;
static const int mul_i64_min_times_minus_one_b = ((int64_t(0x8000000000000000ull) * int64_t(-1)) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int mul_i64_min_times_minus_one_n = ((operator*(int64_t(0x8000000000000000ull), int64_t(-1))) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int mul_i64_minus_one_times_minus_one_b = ((int64_t(-1) * int64_t(-1)) == int64_t(1)) ? 1 : 0;
static const int mul_i64_minus_one_times_minus_one_n = ((operator*(int64_t(-1), int64_t(-1))) == int64_t(1)) ? 1 : 0;
static const int mul_i64_min_times_min_b = ((int64_t(0x8000000000000000ull) * int64_t(0x8000000000000000ull)) == int64_t(0)) ? 1 : 0;
static const int mul_i64_min_times_min_n = ((operator*(int64_t(0x8000000000000000ull), int64_t(0x8000000000000000ull))) == int64_t(0)) ? 1 : 0;
static const int mul_i64_max_times_minus_one_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) * int64_t(-1)) == int64_t(0x8000000000000001ull)) ? 1 : 0;
static const int mul_i64_max_times_minus_one_n = ((operator*(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(-1))) == int64_t(0x8000000000000001ull)) ? 1 : 0;
static const int mul_u64_max_times_two_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) * uint64_t(2)) == uint64_t(0xFFFFFFFFFFFFFFFEull)) ? 1 : 0;
static const int mul_u64_max_times_two_n = ((operator*(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(2))) == uint64_t(0xFFFFFFFFFFFFFFFEull)) ? 1 : 0;
static const int mul_u64_half_width_bit_squared_b = ((uint64_t(0x0000000100000000ull) * uint64_t(0x0000000100000000ull)) == uint64_t(0)) ? 1 : 0;
static const int mul_u64_half_width_bit_squared_n = ((operator*(uint64_t(0x0000000100000000ull), uint64_t(0x0000000100000000ull))) == uint64_t(0)) ? 1 : 0;
static const int mul_u64_six_times_seven_b = ((uint64_t(6) * uint64_t(7)) == uint64_t(42)) ? 1 : 0;
static const int mul_u64_six_times_seven_n = ((operator*(uint64_t(6), uint64_t(7))) == uint64_t(42)) ? 1 : 0;
static const int mul_u64_max_times_zero_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) * uint64_t(0)) == uint64_t(0)) ? 1 : 0;
static const int mul_u64_max_times_zero_n = ((operator*(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0))) == uint64_t(0)) ? 1 : 0;
static const int mul_u64_max_times_max_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) * uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(1)) ? 1 : 0;
static const int mul_u64_max_times_max_n = ((operator*(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(1)) ? 1 : 0;

void check_mul()
{
    switch (0) { case 0: case mul_i8_max_times_two_b: break; }
    switch (0) { case 0: case mul_i8_max_times_two_n: break; }
    switch (0) { case 0: case mul_i8_half_width_bit_squared_b: break; }
    switch (0) { case 0: case mul_i8_half_width_bit_squared_n: break; }
    switch (0) { case 0: case mul_i8_six_times_seven_b: break; }
    switch (0) { case 0: case mul_i8_six_times_seven_n: break; }
    switch (0) { case 0: case mul_i8_max_times_zero_b: break; }
    switch (0) { case 0: case mul_i8_max_times_zero_n: break; }
    switch (0) { case 0: case mul_i8_min_times_minus_one_b: break; }
    switch (0) { case 0: case mul_i8_min_times_minus_one_n: break; }
    switch (0) { case 0: case mul_i8_minus_one_times_minus_one_b: break; }
    switch (0) { case 0: case mul_i8_minus_one_times_minus_one_n: break; }
    switch (0) { case 0: case mul_i8_min_times_min_b: break; }
    switch (0) { case 0: case mul_i8_min_times_min_n: break; }
    switch (0) { case 0: case mul_i8_max_times_minus_one_b: break; }
    switch (0) { case 0: case mul_i8_max_times_minus_one_n: break; }
    switch (0) { case 0: case mul_u8_max_times_two_b: break; }
    switch (0) { case 0: case mul_u8_max_times_two_n: break; }
    switch (0) { case 0: case mul_u8_half_width_bit_squared_b: break; }
    switch (0) { case 0: case mul_u8_half_width_bit_squared_n: break; }
    switch (0) { case 0: case mul_u8_six_times_seven_b: break; }
    switch (0) { case 0: case mul_u8_six_times_seven_n: break; }
    switch (0) { case 0: case mul_u8_max_times_zero_b: break; }
    switch (0) { case 0: case mul_u8_max_times_zero_n: break; }
    switch (0) { case 0: case mul_u8_max_times_max_b: break; }
    switch (0) { case 0: case mul_u8_max_times_max_n: break; }
    switch (0) { case 0: case mul_i64_max_times_two_b: break; }
    switch (0) { case 0: case mul_i64_max_times_two_n: break; }
    switch (0) { case 0: case mul_i64_half_width_bit_squared_b: break; }
    switch (0) { case 0: case mul_i64_half_width_bit_squared_n: break; }
    switch (0) { case 0: case mul_i64_six_times_seven_b: break; }
    switch (0) { case 0: case mul_i64_six_times_seven_n: break; }
    switch (0) { case 0: case mul_i64_max_times_zero_b: break; }
    switch (0) { case 0: case mul_i64_max_times_zero_n: break; }
    switch (0) { case 0: case mul_i64_min_times_minus_one_b: break; }
    switch (0) { case 0: case mul_i64_min_times_minus_one_n: break; }
    switch (0) { case 0: case mul_i64_minus_one_times_minus_one_b: break; }
    switch (0) { case 0: case mul_i64_minus_one_times_minus_one_n: break; }
    switch (0) { case 0: case mul_i64_min_times_min_b: break; }
    switch (0) { case 0: case mul_i64_min_times_min_n: break; }
    switch (0) { case 0: case mul_i64_max_times_minus_one_b: break; }
    switch (0) { case 0: case mul_i64_max_times_minus_one_n: break; }
    switch (0) { case 0: case mul_u64_max_times_two_b: break; }
    switch (0) { case 0: case mul_u64_max_times_two_n: break; }
    switch (0) { case 0: case mul_u64_half_width_bit_squared_b: break; }
    switch (0) { case 0: case mul_u64_half_width_bit_squared_n: break; }
    switch (0) { case 0: case mul_u64_six_times_seven_b: break; }
    switch (0) { case 0: case mul_u64_six_times_seven_n: break; }
    switch (0) { case 0: case mul_u64_max_times_zero_b: break; }
    switch (0) { case 0: case mul_u64_max_times_zero_n: break; }
    switch (0) { case 0: case mul_u64_max_times_max_b: break; }
    switch (0) { case 0: case mul_u64_max_times_max_n: break; }
}

// ------------------------------------------------------------------------------------------------
// div `/`
// Signed `MIN / -1` is target-defined and lives in the diagnostics tests.
// ------------------------------------------------------------------------------------------------

static const int div_i8_seven_by_two_b = ((int8_t(7) / int8_t(2)) == int8_t(3)) ? 1 : 0;
static const int div_i8_seven_by_two_n = ((operator/(int8_t(7), int8_t(2))) == int8_t(3)) ? 1 : 0;
static const int div_i8_max_by_two_b = ((int8_t(127) / int8_t(2)) == int8_t(63)) ? 1 : 0;
static const int div_i8_max_by_two_n = ((operator/(int8_t(127), int8_t(2))) == int8_t(63)) ? 1 : 0;
static const int div_i8_max_by_max_b = ((int8_t(127) / int8_t(127)) == int8_t(1)) ? 1 : 0;
static const int div_i8_max_by_max_n = ((operator/(int8_t(127), int8_t(127))) == int8_t(1)) ? 1 : 0;
static const int div_i8_zero_by_max_b = ((int8_t(0) / int8_t(127)) == int8_t(0)) ? 1 : 0;
static const int div_i8_zero_by_max_n = ((operator/(int8_t(0), int8_t(127))) == int8_t(0)) ? 1 : 0;
static const int div_i8_minus_seven_by_two_b = ((int8_t(-7) / int8_t(2)) == int8_t(-3)) ? 1 : 0;
static const int div_i8_minus_seven_by_two_n = ((operator/(int8_t(-7), int8_t(2))) == int8_t(-3)) ? 1 : 0;
static const int div_i8_seven_by_minus_two_b = ((int8_t(7) / int8_t(-2)) == int8_t(-3)) ? 1 : 0;
static const int div_i8_seven_by_minus_two_n = ((operator/(int8_t(7), int8_t(-2))) == int8_t(-3)) ? 1 : 0;
static const int div_i8_minus_seven_by_minus_two_b = ((int8_t(-7) / int8_t(-2)) == int8_t(3)) ? 1 : 0;
static const int div_i8_minus_seven_by_minus_two_n = ((operator/(int8_t(-7), int8_t(-2))) == int8_t(3)) ? 1 : 0;
static const int div_i8_min_by_one_b = ((int8_t(-128) / int8_t(1)) == int8_t(-128)) ? 1 : 0;
static const int div_i8_min_by_one_n = ((operator/(int8_t(-128), int8_t(1))) == int8_t(-128)) ? 1 : 0;
static const int div_i8_max_by_minus_one_b = ((int8_t(127) / int8_t(-1)) == int8_t(-127)) ? 1 : 0;
static const int div_i8_max_by_minus_one_n = ((operator/(int8_t(127), int8_t(-1))) == int8_t(-127)) ? 1 : 0;
static const int div_i8_min_by_max_b = ((int8_t(-128) / int8_t(127)) == int8_t(-1)) ? 1 : 0;
static const int div_i8_min_by_max_n = ((operator/(int8_t(-128), int8_t(127))) == int8_t(-1)) ? 1 : 0;
static const int div_i8_min_by_min_b = ((int8_t(-128) / int8_t(-128)) == int8_t(1)) ? 1 : 0;
static const int div_i8_min_by_min_n = ((operator/(int8_t(-128), int8_t(-128))) == int8_t(1)) ? 1 : 0;
static const int div_u8_seven_by_two_b = ((uint8_t(0x07) / uint8_t(0x02)) == uint8_t(0x03)) ? 1 : 0;
static const int div_u8_seven_by_two_n = ((operator/(uint8_t(0x07), uint8_t(0x02))) == uint8_t(0x03)) ? 1 : 0;
static const int div_u8_max_by_two_b = ((uint8_t(0xFF) / uint8_t(0x02)) == uint8_t(0x7F)) ? 1 : 0;
static const int div_u8_max_by_two_n = ((operator/(uint8_t(0xFF), uint8_t(0x02))) == uint8_t(0x7F)) ? 1 : 0;
static const int div_u8_max_by_max_b = ((uint8_t(0xFF) / uint8_t(0xFF)) == uint8_t(0x01)) ? 1 : 0;
static const int div_u8_max_by_max_n = ((operator/(uint8_t(0xFF), uint8_t(0xFF))) == uint8_t(0x01)) ? 1 : 0;
static const int div_u8_zero_by_max_b = ((uint8_t(0x00) / uint8_t(0xFF)) == uint8_t(0x00)) ? 1 : 0;
static const int div_u8_zero_by_max_n = ((operator/(uint8_t(0x00), uint8_t(0xFF))) == uint8_t(0x00)) ? 1 : 0;
static const int div_u8_top_bit_by_max_b = ((uint8_t(0x80) / uint8_t(0xFF)) == uint8_t(0x00)) ? 1 : 0;
static const int div_u8_top_bit_by_max_n = ((operator/(uint8_t(0x80), uint8_t(0xFF))) == uint8_t(0x00)) ? 1 : 0;
static const int div_u8_max_by_top_bit_b = ((uint8_t(0xFF) / uint8_t(0x80)) == uint8_t(0x01)) ? 1 : 0;
static const int div_u8_max_by_top_bit_n = ((operator/(uint8_t(0xFF), uint8_t(0x80))) == uint8_t(0x01)) ? 1 : 0;
static const int div_i64_seven_by_two_b = ((int64_t(7) / int64_t(2)) == int64_t(3)) ? 1 : 0;
static const int div_i64_seven_by_two_n = ((operator/(int64_t(7), int64_t(2))) == int64_t(3)) ? 1 : 0;
static const int div_i64_max_by_two_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) / int64_t(2)) == int64_t(0x3FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int div_i64_max_by_two_n = ((operator/(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(2))) == int64_t(0x3FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int div_i64_max_by_max_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) / int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(1)) ? 1 : 0;
static const int div_i64_max_by_max_n = ((operator/(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(1)) ? 1 : 0;
static const int div_i64_zero_by_max_b = ((int64_t(0) / int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(0)) ? 1 : 0;
static const int div_i64_zero_by_max_n = ((operator/(int64_t(0), int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(0)) ? 1 : 0;
static const int div_i64_minus_seven_by_two_b = ((int64_t(-7) / int64_t(2)) == int64_t(-3)) ? 1 : 0;
static const int div_i64_minus_seven_by_two_n = ((operator/(int64_t(-7), int64_t(2))) == int64_t(-3)) ? 1 : 0;
static const int div_i64_seven_by_minus_two_b = ((int64_t(7) / int64_t(-2)) == int64_t(-3)) ? 1 : 0;
static const int div_i64_seven_by_minus_two_n = ((operator/(int64_t(7), int64_t(-2))) == int64_t(-3)) ? 1 : 0;
static const int div_i64_minus_seven_by_minus_two_b = ((int64_t(-7) / int64_t(-2)) == int64_t(3)) ? 1 : 0;
static const int div_i64_minus_seven_by_minus_two_n = ((operator/(int64_t(-7), int64_t(-2))) == int64_t(3)) ? 1 : 0;
static const int div_i64_min_by_one_b = ((int64_t(0x8000000000000000ull) / int64_t(1)) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int div_i64_min_by_one_n = ((operator/(int64_t(0x8000000000000000ull), int64_t(1))) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int div_i64_max_by_minus_one_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) / int64_t(-1)) == int64_t(0x8000000000000001ull)) ? 1 : 0;
static const int div_i64_max_by_minus_one_n = ((operator/(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(-1))) == int64_t(0x8000000000000001ull)) ? 1 : 0;
static const int div_i64_min_by_max_b = ((int64_t(0x8000000000000000ull) / int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(-1)) ? 1 : 0;
static const int div_i64_min_by_max_n = ((operator/(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(-1)) ? 1 : 0;
static const int div_i64_min_by_min_b = ((int64_t(0x8000000000000000ull) / int64_t(0x8000000000000000ull)) == int64_t(1)) ? 1 : 0;
static const int div_i64_min_by_min_n = ((operator/(int64_t(0x8000000000000000ull), int64_t(0x8000000000000000ull))) == int64_t(1)) ? 1 : 0;
static const int div_u64_seven_by_two_b = ((uint64_t(7) / uint64_t(2)) == uint64_t(3)) ? 1 : 0;
static const int div_u64_seven_by_two_n = ((operator/(uint64_t(7), uint64_t(2))) == uint64_t(3)) ? 1 : 0;
static const int div_u64_max_by_two_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) / uint64_t(2)) == uint64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int div_u64_max_by_two_n = ((operator/(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(2))) == uint64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int div_u64_max_by_max_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) / uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(1)) ? 1 : 0;
static const int div_u64_max_by_max_n = ((operator/(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(1)) ? 1 : 0;
static const int div_u64_zero_by_max_b = ((uint64_t(0) / uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0)) ? 1 : 0;
static const int div_u64_zero_by_max_n = ((operator/(uint64_t(0), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(0)) ? 1 : 0;
static const int div_u64_top_bit_by_max_b = ((uint64_t(0x8000000000000000ull) / uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0)) ? 1 : 0;
static const int div_u64_top_bit_by_max_n = ((operator/(uint64_t(0x8000000000000000ull), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(0)) ? 1 : 0;
static const int div_u64_max_by_top_bit_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) / uint64_t(0x8000000000000000ull)) == uint64_t(1)) ? 1 : 0;
static const int div_u64_max_by_top_bit_n = ((operator/(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0x8000000000000000ull))) == uint64_t(1)) ? 1 : 0;

void check_div()
{
    switch (0) { case 0: case div_i8_seven_by_two_b: break; }
    switch (0) { case 0: case div_i8_seven_by_two_n: break; }
    switch (0) { case 0: case div_i8_max_by_two_b: break; }
    switch (0) { case 0: case div_i8_max_by_two_n: break; }
    switch (0) { case 0: case div_i8_max_by_max_b: break; }
    switch (0) { case 0: case div_i8_max_by_max_n: break; }
    switch (0) { case 0: case div_i8_zero_by_max_b: break; }
    switch (0) { case 0: case div_i8_zero_by_max_n: break; }
    switch (0) { case 0: case div_i8_minus_seven_by_two_b: break; }
    switch (0) { case 0: case div_i8_minus_seven_by_two_n: break; }
    switch (0) { case 0: case div_i8_seven_by_minus_two_b: break; }
    switch (0) { case 0: case div_i8_seven_by_minus_two_n: break; }
    switch (0) { case 0: case div_i8_minus_seven_by_minus_two_b: break; }
    switch (0) { case 0: case div_i8_minus_seven_by_minus_two_n: break; }
    switch (0) { case 0: case div_i8_min_by_one_b: break; }
    switch (0) { case 0: case div_i8_min_by_one_n: break; }
    switch (0) { case 0: case div_i8_max_by_minus_one_b: break; }
    switch (0) { case 0: case div_i8_max_by_minus_one_n: break; }
    switch (0) { case 0: case div_i8_min_by_max_b: break; }
    switch (0) { case 0: case div_i8_min_by_max_n: break; }
    switch (0) { case 0: case div_i8_min_by_min_b: break; }
    switch (0) { case 0: case div_i8_min_by_min_n: break; }
    switch (0) { case 0: case div_u8_seven_by_two_b: break; }
    switch (0) { case 0: case div_u8_seven_by_two_n: break; }
    switch (0) { case 0: case div_u8_max_by_two_b: break; }
    switch (0) { case 0: case div_u8_max_by_two_n: break; }
    switch (0) { case 0: case div_u8_max_by_max_b: break; }
    switch (0) { case 0: case div_u8_max_by_max_n: break; }
    switch (0) { case 0: case div_u8_zero_by_max_b: break; }
    switch (0) { case 0: case div_u8_zero_by_max_n: break; }
    switch (0) { case 0: case div_u8_top_bit_by_max_b: break; }
    switch (0) { case 0: case div_u8_top_bit_by_max_n: break; }
    switch (0) { case 0: case div_u8_max_by_top_bit_b: break; }
    switch (0) { case 0: case div_u8_max_by_top_bit_n: break; }
    switch (0) { case 0: case div_i64_seven_by_two_b: break; }
    switch (0) { case 0: case div_i64_seven_by_two_n: break; }
    switch (0) { case 0: case div_i64_max_by_two_b: break; }
    switch (0) { case 0: case div_i64_max_by_two_n: break; }
    switch (0) { case 0: case div_i64_max_by_max_b: break; }
    switch (0) { case 0: case div_i64_max_by_max_n: break; }
    switch (0) { case 0: case div_i64_zero_by_max_b: break; }
    switch (0) { case 0: case div_i64_zero_by_max_n: break; }
    switch (0) { case 0: case div_i64_minus_seven_by_two_b: break; }
    switch (0) { case 0: case div_i64_minus_seven_by_two_n: break; }
    switch (0) { case 0: case div_i64_seven_by_minus_two_b: break; }
    switch (0) { case 0: case div_i64_seven_by_minus_two_n: break; }
    switch (0) { case 0: case div_i64_minus_seven_by_minus_two_b: break; }
    switch (0) { case 0: case div_i64_minus_seven_by_minus_two_n: break; }
    switch (0) { case 0: case div_i64_min_by_one_b: break; }
    switch (0) { case 0: case div_i64_min_by_one_n: break; }
    switch (0) { case 0: case div_i64_max_by_minus_one_b: break; }
    switch (0) { case 0: case div_i64_max_by_minus_one_n: break; }
    switch (0) { case 0: case div_i64_min_by_max_b: break; }
    switch (0) { case 0: case div_i64_min_by_max_n: break; }
    switch (0) { case 0: case div_i64_min_by_min_b: break; }
    switch (0) { case 0: case div_i64_min_by_min_n: break; }
    switch (0) { case 0: case div_u64_seven_by_two_b: break; }
    switch (0) { case 0: case div_u64_seven_by_two_n: break; }
    switch (0) { case 0: case div_u64_max_by_two_b: break; }
    switch (0) { case 0: case div_u64_max_by_two_n: break; }
    switch (0) { case 0: case div_u64_max_by_max_b: break; }
    switch (0) { case 0: case div_u64_max_by_max_n: break; }
    switch (0) { case 0: case div_u64_zero_by_max_b: break; }
    switch (0) { case 0: case div_u64_zero_by_max_n: break; }
    switch (0) { case 0: case div_u64_top_bit_by_max_b: break; }
    switch (0) { case 0: case div_u64_top_bit_by_max_n: break; }
    switch (0) { case 0: case div_u64_max_by_top_bit_b: break; }
    switch (0) { case 0: case div_u64_max_by_top_bit_n: break; }
}

// ------------------------------------------------------------------------------------------------
// rem `%`
// The sign of the result follows the dividend. Signed `MIN % -1` is
// target-defined and lives in the diagnostics tests.
// ------------------------------------------------------------------------------------------------

static const int rem_i8_seven_rem_three_b = ((int8_t(7) % int8_t(3)) == int8_t(1)) ? 1 : 0;
static const int rem_i8_seven_rem_three_n = ((operator%(int8_t(7), int8_t(3))) == int8_t(1)) ? 1 : 0;
static const int rem_i8_max_rem_ten_b = ((int8_t(127) % int8_t(10)) == int8_t(7)) ? 1 : 0;
static const int rem_i8_max_rem_ten_n = ((operator%(int8_t(127), int8_t(10))) == int8_t(7)) ? 1 : 0;
static const int rem_i8_max_rem_max_b = ((int8_t(127) % int8_t(127)) == int8_t(0)) ? 1 : 0;
static const int rem_i8_max_rem_max_n = ((operator%(int8_t(127), int8_t(127))) == int8_t(0)) ? 1 : 0;
static const int rem_i8_zero_rem_max_b = ((int8_t(0) % int8_t(127)) == int8_t(0)) ? 1 : 0;
static const int rem_i8_zero_rem_max_n = ((operator%(int8_t(0), int8_t(127))) == int8_t(0)) ? 1 : 0;
static const int rem_i8_minus_seven_rem_three_b = ((int8_t(-7) % int8_t(3)) == int8_t(-1)) ? 1 : 0;
static const int rem_i8_minus_seven_rem_three_n = ((operator%(int8_t(-7), int8_t(3))) == int8_t(-1)) ? 1 : 0;
static const int rem_i8_seven_rem_minus_three_b = ((int8_t(7) % int8_t(-3)) == int8_t(1)) ? 1 : 0;
static const int rem_i8_seven_rem_minus_three_n = ((operator%(int8_t(7), int8_t(-3))) == int8_t(1)) ? 1 : 0;
static const int rem_i8_minus_seven_rem_minus_three_b = ((int8_t(-7) % int8_t(-3)) == int8_t(-1)) ? 1 : 0;
static const int rem_i8_minus_seven_rem_minus_three_n = ((operator%(int8_t(-7), int8_t(-3))) == int8_t(-1)) ? 1 : 0;
static const int rem_i8_min_rem_one_b = ((int8_t(-128) % int8_t(1)) == int8_t(0)) ? 1 : 0;
static const int rem_i8_min_rem_one_n = ((operator%(int8_t(-128), int8_t(1))) == int8_t(0)) ? 1 : 0;
static const int rem_i8_min_rem_max_b = ((int8_t(-128) % int8_t(127)) == int8_t(-1)) ? 1 : 0;
static const int rem_i8_min_rem_max_n = ((operator%(int8_t(-128), int8_t(127))) == int8_t(-1)) ? 1 : 0;
static const int rem_i8_max_rem_minus_one_b = ((int8_t(127) % int8_t(-1)) == int8_t(0)) ? 1 : 0;
static const int rem_i8_max_rem_minus_one_n = ((operator%(int8_t(127), int8_t(-1))) == int8_t(0)) ? 1 : 0;
static const int rem_u8_seven_rem_three_b = ((uint8_t(0x07) % uint8_t(0x03)) == uint8_t(0x01)) ? 1 : 0;
static const int rem_u8_seven_rem_three_n = ((operator%(uint8_t(0x07), uint8_t(0x03))) == uint8_t(0x01)) ? 1 : 0;
static const int rem_u8_max_rem_ten_b = ((uint8_t(0xFF) % uint8_t(0x0A)) == uint8_t(0x05)) ? 1 : 0;
static const int rem_u8_max_rem_ten_n = ((operator%(uint8_t(0xFF), uint8_t(0x0A))) == uint8_t(0x05)) ? 1 : 0;
static const int rem_u8_max_rem_max_b = ((uint8_t(0xFF) % uint8_t(0xFF)) == uint8_t(0x00)) ? 1 : 0;
static const int rem_u8_max_rem_max_n = ((operator%(uint8_t(0xFF), uint8_t(0xFF))) == uint8_t(0x00)) ? 1 : 0;
static const int rem_u8_zero_rem_max_b = ((uint8_t(0x00) % uint8_t(0xFF)) == uint8_t(0x00)) ? 1 : 0;
static const int rem_u8_zero_rem_max_n = ((operator%(uint8_t(0x00), uint8_t(0xFF))) == uint8_t(0x00)) ? 1 : 0;
static const int rem_u8_top_bit_rem_max_b = ((uint8_t(0x80) % uint8_t(0xFF)) == uint8_t(0x80)) ? 1 : 0;
static const int rem_u8_top_bit_rem_max_n = ((operator%(uint8_t(0x80), uint8_t(0xFF))) == uint8_t(0x80)) ? 1 : 0;
static const int rem_u8_max_rem_top_bit_b = ((uint8_t(0xFF) % uint8_t(0x80)) == uint8_t(0x7F)) ? 1 : 0;
static const int rem_u8_max_rem_top_bit_n = ((operator%(uint8_t(0xFF), uint8_t(0x80))) == uint8_t(0x7F)) ? 1 : 0;
static const int rem_i64_seven_rem_three_b = ((int64_t(7) % int64_t(3)) == int64_t(1)) ? 1 : 0;
static const int rem_i64_seven_rem_three_n = ((operator%(int64_t(7), int64_t(3))) == int64_t(1)) ? 1 : 0;
static const int rem_i64_max_rem_ten_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) % int64_t(10)) == int64_t(7)) ? 1 : 0;
static const int rem_i64_max_rem_ten_n = ((operator%(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(10))) == int64_t(7)) ? 1 : 0;
static const int rem_i64_max_rem_max_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) % int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(0)) ? 1 : 0;
static const int rem_i64_max_rem_max_n = ((operator%(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(0)) ? 1 : 0;
static const int rem_i64_zero_rem_max_b = ((int64_t(0) % int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(0)) ? 1 : 0;
static const int rem_i64_zero_rem_max_n = ((operator%(int64_t(0), int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(0)) ? 1 : 0;
static const int rem_i64_minus_seven_rem_three_b = ((int64_t(-7) % int64_t(3)) == int64_t(-1)) ? 1 : 0;
static const int rem_i64_minus_seven_rem_three_n = ((operator%(int64_t(-7), int64_t(3))) == int64_t(-1)) ? 1 : 0;
static const int rem_i64_seven_rem_minus_three_b = ((int64_t(7) % int64_t(-3)) == int64_t(1)) ? 1 : 0;
static const int rem_i64_seven_rem_minus_three_n = ((operator%(int64_t(7), int64_t(-3))) == int64_t(1)) ? 1 : 0;
static const int rem_i64_minus_seven_rem_minus_three_b = ((int64_t(-7) % int64_t(-3)) == int64_t(-1)) ? 1 : 0;
static const int rem_i64_minus_seven_rem_minus_three_n = ((operator%(int64_t(-7), int64_t(-3))) == int64_t(-1)) ? 1 : 0;
static const int rem_i64_min_rem_one_b = ((int64_t(0x8000000000000000ull) % int64_t(1)) == int64_t(0)) ? 1 : 0;
static const int rem_i64_min_rem_one_n = ((operator%(int64_t(0x8000000000000000ull), int64_t(1))) == int64_t(0)) ? 1 : 0;
static const int rem_i64_min_rem_max_b = ((int64_t(0x8000000000000000ull) % int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(-1)) ? 1 : 0;
static const int rem_i64_min_rem_max_n = ((operator%(int64_t(0x8000000000000000ull), int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(-1)) ? 1 : 0;
static const int rem_i64_max_rem_minus_one_b = ((int64_t(0x7FFFFFFFFFFFFFFFull) % int64_t(-1)) == int64_t(0)) ? 1 : 0;
static const int rem_i64_max_rem_minus_one_n = ((operator%(int64_t(0x7FFFFFFFFFFFFFFFull), int64_t(-1))) == int64_t(0)) ? 1 : 0;
static const int rem_u64_seven_rem_three_b = ((uint64_t(7) % uint64_t(3)) == uint64_t(1)) ? 1 : 0;
static const int rem_u64_seven_rem_three_n = ((operator%(uint64_t(7), uint64_t(3))) == uint64_t(1)) ? 1 : 0;
static const int rem_u64_max_rem_ten_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) % uint64_t(10)) == uint64_t(5)) ? 1 : 0;
static const int rem_u64_max_rem_ten_n = ((operator%(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(10))) == uint64_t(5)) ? 1 : 0;
static const int rem_u64_max_rem_max_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) % uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0)) ? 1 : 0;
static const int rem_u64_max_rem_max_n = ((operator%(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(0)) ? 1 : 0;
static const int rem_u64_zero_rem_max_b = ((uint64_t(0) % uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0)) ? 1 : 0;
static const int rem_u64_zero_rem_max_n = ((operator%(uint64_t(0), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(0)) ? 1 : 0;
static const int rem_u64_top_bit_rem_max_b = ((uint64_t(0x8000000000000000ull) % uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0x8000000000000000ull)) ? 1 : 0;
static const int rem_u64_top_bit_rem_max_n = ((operator%(uint64_t(0x8000000000000000ull), uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(0x8000000000000000ull)) ? 1 : 0;
static const int rem_u64_max_rem_top_bit_b = ((uint64_t(0xFFFFFFFFFFFFFFFFull) % uint64_t(0x8000000000000000ull)) == uint64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int rem_u64_max_rem_top_bit_n = ((operator%(uint64_t(0xFFFFFFFFFFFFFFFFull), uint64_t(0x8000000000000000ull))) == uint64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;

void check_rem()
{
    switch (0) { case 0: case rem_i8_seven_rem_three_b: break; }
    switch (0) { case 0: case rem_i8_seven_rem_three_n: break; }
    switch (0) { case 0: case rem_i8_max_rem_ten_b: break; }
    switch (0) { case 0: case rem_i8_max_rem_ten_n: break; }
    switch (0) { case 0: case rem_i8_max_rem_max_b: break; }
    switch (0) { case 0: case rem_i8_max_rem_max_n: break; }
    switch (0) { case 0: case rem_i8_zero_rem_max_b: break; }
    switch (0) { case 0: case rem_i8_zero_rem_max_n: break; }
    switch (0) { case 0: case rem_i8_minus_seven_rem_three_b: break; }
    switch (0) { case 0: case rem_i8_minus_seven_rem_three_n: break; }
    switch (0) { case 0: case rem_i8_seven_rem_minus_three_b: break; }
    switch (0) { case 0: case rem_i8_seven_rem_minus_three_n: break; }
    switch (0) { case 0: case rem_i8_minus_seven_rem_minus_three_b: break; }
    switch (0) { case 0: case rem_i8_minus_seven_rem_minus_three_n: break; }
    switch (0) { case 0: case rem_i8_min_rem_one_b: break; }
    switch (0) { case 0: case rem_i8_min_rem_one_n: break; }
    switch (0) { case 0: case rem_i8_min_rem_max_b: break; }
    switch (0) { case 0: case rem_i8_min_rem_max_n: break; }
    switch (0) { case 0: case rem_i8_max_rem_minus_one_b: break; }
    switch (0) { case 0: case rem_i8_max_rem_minus_one_n: break; }
    switch (0) { case 0: case rem_u8_seven_rem_three_b: break; }
    switch (0) { case 0: case rem_u8_seven_rem_three_n: break; }
    switch (0) { case 0: case rem_u8_max_rem_ten_b: break; }
    switch (0) { case 0: case rem_u8_max_rem_ten_n: break; }
    switch (0) { case 0: case rem_u8_max_rem_max_b: break; }
    switch (0) { case 0: case rem_u8_max_rem_max_n: break; }
    switch (0) { case 0: case rem_u8_zero_rem_max_b: break; }
    switch (0) { case 0: case rem_u8_zero_rem_max_n: break; }
    switch (0) { case 0: case rem_u8_top_bit_rem_max_b: break; }
    switch (0) { case 0: case rem_u8_top_bit_rem_max_n: break; }
    switch (0) { case 0: case rem_u8_max_rem_top_bit_b: break; }
    switch (0) { case 0: case rem_u8_max_rem_top_bit_n: break; }
    switch (0) { case 0: case rem_i64_seven_rem_three_b: break; }
    switch (0) { case 0: case rem_i64_seven_rem_three_n: break; }
    switch (0) { case 0: case rem_i64_max_rem_ten_b: break; }
    switch (0) { case 0: case rem_i64_max_rem_ten_n: break; }
    switch (0) { case 0: case rem_i64_max_rem_max_b: break; }
    switch (0) { case 0: case rem_i64_max_rem_max_n: break; }
    switch (0) { case 0: case rem_i64_zero_rem_max_b: break; }
    switch (0) { case 0: case rem_i64_zero_rem_max_n: break; }
    switch (0) { case 0: case rem_i64_minus_seven_rem_three_b: break; }
    switch (0) { case 0: case rem_i64_minus_seven_rem_three_n: break; }
    switch (0) { case 0: case rem_i64_seven_rem_minus_three_b: break; }
    switch (0) { case 0: case rem_i64_seven_rem_minus_three_n: break; }
    switch (0) { case 0: case rem_i64_minus_seven_rem_minus_three_b: break; }
    switch (0) { case 0: case rem_i64_minus_seven_rem_minus_three_n: break; }
    switch (0) { case 0: case rem_i64_min_rem_one_b: break; }
    switch (0) { case 0: case rem_i64_min_rem_one_n: break; }
    switch (0) { case 0: case rem_i64_min_rem_max_b: break; }
    switch (0) { case 0: case rem_i64_min_rem_max_n: break; }
    switch (0) { case 0: case rem_i64_max_rem_minus_one_b: break; }
    switch (0) { case 0: case rem_i64_max_rem_minus_one_n: break; }
    switch (0) { case 0: case rem_u64_seven_rem_three_b: break; }
    switch (0) { case 0: case rem_u64_seven_rem_three_n: break; }
    switch (0) { case 0: case rem_u64_max_rem_ten_b: break; }
    switch (0) { case 0: case rem_u64_max_rem_ten_n: break; }
    switch (0) { case 0: case rem_u64_max_rem_max_b: break; }
    switch (0) { case 0: case rem_u64_max_rem_max_n: break; }
    switch (0) { case 0: case rem_u64_zero_rem_max_b: break; }
    switch (0) { case 0: case rem_u64_zero_rem_max_n: break; }
    switch (0) { case 0: case rem_u64_top_bit_rem_max_b: break; }
    switch (0) { case 0: case rem_u64_top_bit_rem_max_n: break; }
    switch (0) { case 0: case rem_u64_max_rem_top_bit_b: break; }
    switch (0) { case 0: case rem_u64_max_rem_top_bit_n: break; }
}

// ------------------------------------------------------------------------------------------------
// neg unary `-`
// `-MIN` wraps to `MIN` without a diagnostic.
// ------------------------------------------------------------------------------------------------

static const int neg_i8_zero_b = ((-int8_t(0)) == int8_t(0)) ? 1 : 0;
static const int neg_i8_zero_n = ((operator-(int8_t(0))) == int8_t(0)) ? 1 : 0;
static const int neg_i8_one_b = ((-int8_t(1)) == int8_t(-1)) ? 1 : 0;
static const int neg_i8_one_n = ((operator-(int8_t(1))) == int8_t(-1)) ? 1 : 0;
static const int neg_i8_max_b = ((-int8_t(127)) == int8_t(-127)) ? 1 : 0;
static const int neg_i8_max_n = ((operator-(int8_t(127))) == int8_t(-127)) ? 1 : 0;
static const int neg_i8_min_b = ((-int8_t(-128)) == int8_t(-128)) ? 1 : 0;
static const int neg_i8_min_n = ((operator-(int8_t(-128))) == int8_t(-128)) ? 1 : 0;
static const int neg_i8_minus_one_b = ((-int8_t(-1)) == int8_t(1)) ? 1 : 0;
static const int neg_i8_minus_one_n = ((operator-(int8_t(-1))) == int8_t(1)) ? 1 : 0;
static const int neg_u8_zero_b = ((-uint8_t(0x00)) == uint8_t(0x00)) ? 1 : 0;
static const int neg_u8_zero_n = ((operator-(uint8_t(0x00))) == uint8_t(0x00)) ? 1 : 0;
static const int neg_u8_one_b = ((-uint8_t(0x01)) == uint8_t(0xFF)) ? 1 : 0;
static const int neg_u8_one_n = ((operator-(uint8_t(0x01))) == uint8_t(0xFF)) ? 1 : 0;
static const int neg_u8_max_b = ((-uint8_t(0xFF)) == uint8_t(0x01)) ? 1 : 0;
static const int neg_u8_max_n = ((operator-(uint8_t(0xFF))) == uint8_t(0x01)) ? 1 : 0;
static const int neg_u8_top_bit_b = ((-uint8_t(0x80)) == uint8_t(0x80)) ? 1 : 0;
static const int neg_u8_top_bit_n = ((operator-(uint8_t(0x80))) == uint8_t(0x80)) ? 1 : 0;
static const int neg_i64_zero_b = ((-int64_t(0)) == int64_t(0)) ? 1 : 0;
static const int neg_i64_zero_n = ((operator-(int64_t(0))) == int64_t(0)) ? 1 : 0;
static const int neg_i64_one_b = ((-int64_t(1)) == int64_t(-1)) ? 1 : 0;
static const int neg_i64_one_n = ((operator-(int64_t(1))) == int64_t(-1)) ? 1 : 0;
static const int neg_i64_max_b = ((-int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(0x8000000000000001ull)) ? 1 : 0;
static const int neg_i64_max_n = ((operator-(int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(0x8000000000000001ull)) ? 1 : 0;
static const int neg_i64_min_b = ((-int64_t(0x8000000000000000ull)) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int neg_i64_min_n = ((operator-(int64_t(0x8000000000000000ull))) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int neg_i64_minus_one_b = ((-int64_t(-1)) == int64_t(1)) ? 1 : 0;
static const int neg_i64_minus_one_n = ((operator-(int64_t(-1))) == int64_t(1)) ? 1 : 0;
static const int neg_u64_zero_b = ((-uint64_t(0)) == uint64_t(0)) ? 1 : 0;
static const int neg_u64_zero_n = ((operator-(uint64_t(0))) == uint64_t(0)) ? 1 : 0;
static const int neg_u64_one_b = ((-uint64_t(1)) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int neg_u64_one_n = ((operator-(uint64_t(1))) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int neg_u64_max_b = ((-uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(1)) ? 1 : 0;
static const int neg_u64_max_n = ((operator-(uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(1)) ? 1 : 0;
static const int neg_u64_top_bit_b = ((-uint64_t(0x8000000000000000ull)) == uint64_t(0x8000000000000000ull)) ? 1 : 0;
static const int neg_u64_top_bit_n = ((operator-(uint64_t(0x8000000000000000ull))) == uint64_t(0x8000000000000000ull)) ? 1 : 0;

void check_neg()
{
    switch (0) { case 0: case neg_i8_zero_b: break; }
    switch (0) { case 0: case neg_i8_zero_n: break; }
    switch (0) { case 0: case neg_i8_one_b: break; }
    switch (0) { case 0: case neg_i8_one_n: break; }
    switch (0) { case 0: case neg_i8_max_b: break; }
    switch (0) { case 0: case neg_i8_max_n: break; }
    switch (0) { case 0: case neg_i8_min_b: break; }
    switch (0) { case 0: case neg_i8_min_n: break; }
    switch (0) { case 0: case neg_i8_minus_one_b: break; }
    switch (0) { case 0: case neg_i8_minus_one_n: break; }
    switch (0) { case 0: case neg_u8_zero_b: break; }
    switch (0) { case 0: case neg_u8_zero_n: break; }
    switch (0) { case 0: case neg_u8_one_b: break; }
    switch (0) { case 0: case neg_u8_one_n: break; }
    switch (0) { case 0: case neg_u8_max_b: break; }
    switch (0) { case 0: case neg_u8_max_n: break; }
    switch (0) { case 0: case neg_u8_top_bit_b: break; }
    switch (0) { case 0: case neg_u8_top_bit_n: break; }
    switch (0) { case 0: case neg_i64_zero_b: break; }
    switch (0) { case 0: case neg_i64_zero_n: break; }
    switch (0) { case 0: case neg_i64_one_b: break; }
    switch (0) { case 0: case neg_i64_one_n: break; }
    switch (0) { case 0: case neg_i64_max_b: break; }
    switch (0) { case 0: case neg_i64_max_n: break; }
    switch (0) { case 0: case neg_i64_min_b: break; }
    switch (0) { case 0: case neg_i64_min_n: break; }
    switch (0) { case 0: case neg_i64_minus_one_b: break; }
    switch (0) { case 0: case neg_i64_minus_one_n: break; }
    switch (0) { case 0: case neg_u64_zero_b: break; }
    switch (0) { case 0: case neg_u64_zero_n: break; }
    switch (0) { case 0: case neg_u64_one_b: break; }
    switch (0) { case 0: case neg_u64_one_n: break; }
    switch (0) { case 0: case neg_u64_max_b: break; }
    switch (0) { case 0: case neg_u64_max_n: break; }
    switch (0) { case 0: case neg_u64_top_bit_b: break; }
    switch (0) { case 0: case neg_u64_top_bit_n: break; }
}

// ------------------------------------------------------------------------------------------------
// pos unary `+`
// Unary `+` is not rewritten to a `BuiltinOperatorExpr`, so both spellings
// take the named path: `_infix` is `+x` and `_call` is `operator+(x)`.
// ------------------------------------------------------------------------------------------------

static const int pos_i8_min_infix = ((+int8_t(-128)) == int8_t(-128)) ? 1 : 0;
static const int pos_i8_min_call = ((operator+(int8_t(-128))) == int8_t(-128)) ? 1 : 0;
static const int pos_i8_max_infix = ((+int8_t(127)) == int8_t(127)) ? 1 : 0;
static const int pos_i8_max_call = ((operator+(int8_t(127))) == int8_t(127)) ? 1 : 0;
static const int pos_i8_five_infix = ((+int8_t(5)) == int8_t(5)) ? 1 : 0;
static const int pos_i8_five_call = ((operator+(int8_t(5))) == int8_t(5)) ? 1 : 0;
static const int pos_i8_minus_five_infix = ((+int8_t(-5)) == int8_t(-5)) ? 1 : 0;
static const int pos_i8_minus_five_call = ((operator+(int8_t(-5))) == int8_t(-5)) ? 1 : 0;
static const int pos_u8_min_infix = ((+uint8_t(0x00)) == uint8_t(0x00)) ? 1 : 0;
static const int pos_u8_min_call = ((operator+(uint8_t(0x00))) == uint8_t(0x00)) ? 1 : 0;
static const int pos_u8_max_infix = ((+uint8_t(0xFF)) == uint8_t(0xFF)) ? 1 : 0;
static const int pos_u8_max_call = ((operator+(uint8_t(0xFF))) == uint8_t(0xFF)) ? 1 : 0;
static const int pos_u8_five_infix = ((+uint8_t(0x05)) == uint8_t(0x05)) ? 1 : 0;
static const int pos_u8_five_call = ((operator+(uint8_t(0x05))) == uint8_t(0x05)) ? 1 : 0;
static const int pos_i64_min_infix = ((+int64_t(0x8000000000000000ull)) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int pos_i64_min_call = ((operator+(int64_t(0x8000000000000000ull))) == int64_t(0x8000000000000000ull)) ? 1 : 0;
static const int pos_i64_max_infix = ((+int64_t(0x7FFFFFFFFFFFFFFFull)) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int pos_i64_max_call = ((operator+(int64_t(0x7FFFFFFFFFFFFFFFull))) == int64_t(0x7FFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int pos_i64_five_infix = ((+int64_t(5)) == int64_t(5)) ? 1 : 0;
static const int pos_i64_five_call = ((operator+(int64_t(5))) == int64_t(5)) ? 1 : 0;
static const int pos_i64_minus_five_infix = ((+int64_t(-5)) == int64_t(-5)) ? 1 : 0;
static const int pos_i64_minus_five_call = ((operator+(int64_t(-5))) == int64_t(-5)) ? 1 : 0;
static const int pos_u64_min_infix = ((+uint64_t(0)) == uint64_t(0)) ? 1 : 0;
static const int pos_u64_min_call = ((operator+(uint64_t(0))) == uint64_t(0)) ? 1 : 0;
static const int pos_u64_max_infix = ((+uint64_t(0xFFFFFFFFFFFFFFFFull)) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int pos_u64_max_call = ((operator+(uint64_t(0xFFFFFFFFFFFFFFFFull))) == uint64_t(0xFFFFFFFFFFFFFFFFull)) ? 1 : 0;
static const int pos_u64_five_infix = ((+uint64_t(5)) == uint64_t(5)) ? 1 : 0;
static const int pos_u64_five_call = ((operator+(uint64_t(5))) == uint64_t(5)) ? 1 : 0;

void check_pos()
{
    switch (0) { case 0: case pos_i8_min_infix: break; }
    switch (0) { case 0: case pos_i8_min_call: break; }
    switch (0) { case 0: case pos_i8_max_infix: break; }
    switch (0) { case 0: case pos_i8_max_call: break; }
    switch (0) { case 0: case pos_i8_five_infix: break; }
    switch (0) { case 0: case pos_i8_five_call: break; }
    switch (0) { case 0: case pos_i8_minus_five_infix: break; }
    switch (0) { case 0: case pos_i8_minus_five_call: break; }
    switch (0) { case 0: case pos_u8_min_infix: break; }
    switch (0) { case 0: case pos_u8_min_call: break; }
    switch (0) { case 0: case pos_u8_max_infix: break; }
    switch (0) { case 0: case pos_u8_max_call: break; }
    switch (0) { case 0: case pos_u8_five_infix: break; }
    switch (0) { case 0: case pos_u8_five_call: break; }
    switch (0) { case 0: case pos_i64_min_infix: break; }
    switch (0) { case 0: case pos_i64_min_call: break; }
    switch (0) { case 0: case pos_i64_max_infix: break; }
    switch (0) { case 0: case pos_i64_max_call: break; }
    switch (0) { case 0: case pos_i64_five_infix: break; }
    switch (0) { case 0: case pos_i64_five_call: break; }
    switch (0) { case 0: case pos_i64_minus_five_infix: break; }
    switch (0) { case 0: case pos_i64_minus_five_call: break; }
    switch (0) { case 0: case pos_u64_min_infix: break; }
    switch (0) { case 0: case pos_u64_min_call: break; }
    switch (0) { case 0: case pos_u64_max_infix: break; }
    switch (0) { case 0: case pos_u64_max_call: break; }
    switch (0) { case 0: case pos_u64_five_infix: break; }
    switch (0) { case 0: case pos_u64_five_call: break; }
}
