//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

// Front-end constant folding of an explicit cast of a floating-point literal to an integer type.
// The value truncates toward zero and must fit in the destination type.
// `tests/diagnostics/float-to-int-cast-diagnostics.slang` covers the values that do not fit.
//
// Each case is a `static const int` that is 1 when the folded expression has the expected
// value and 0 otherwise. A `switch` then uses the constant as a second case label next to
// `case 0`, which makes the front end fold it to a concrete value:
//
// - a wrong result folds to 0 and fails with "duplicate cases in switch" at the `case` line,
// - a result that does not fold to a concrete value fails with "expression does not evaluate to
//   a compile-time constant".
//
// The narrow results are widened before the comparison, because a comparison in the narrow
// type would narrow both operands first.

static const int int_truncates_positive = ((int)7.9f == 7) ? 1 : 0;
static const int int_truncates_negative = ((int)-7.9f == -7) ? 1 : 0;
static const int int8_min_edge = (int((int8_t)-128.9f) == -128) ? 1 : 0;
static const int int8_max_edge = (int((int8_t)127.9f) == 127) ? 1 : 0;
static const int uint8_max_edge = (uint((uint8_t)255.5f) == 255u) ? 1 : 0;
static const int uint8_negative_fraction = (uint((uint8_t)-0.5f) == 0u) ? 1 : 0;
static const int uint_max_edge = ((uint)4294967295.0l == 0xFFFFFFFFu) ? 1 : 0;

// 2^63 fits in `uint64_t`, and -2^63 fits in `int64_t`.
static const int uint64_top_bit = ((uint64_t)9223372036854775808.0 == 0x8000000000000000ull) ? 1 : 0;
static const int int64_min_edge =
    ((int64_t)-9223372036854775808.0 == int64_t(0x8000000000000000ull)) ? 1 : 0;

void use()
{
    switch (0) { case 0: case int_truncates_positive: break; }
    switch (0) { case 0: case int_truncates_negative: break; }
    switch (0) { case 0: case int8_min_edge: break; }
    switch (0) { case 0: case int8_max_edge: break; }
    switch (0) { case 0: case uint8_max_edge: break; }
    switch (0) { case 0: case uint8_negative_fraction: break; }
    switch (0) { case 0: case uint_max_edge: break; }
    switch (0) { case 0: case uint64_top_bit: break; }
    switch (0) { case 0: case int64_min_edge: break; }
}
