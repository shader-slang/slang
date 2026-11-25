//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

// The parser folds a prefix `~`, `-` or `+` applied to an integer literal into a new literal. The
// folded value must fit the literal's type, which the suffix determines. For example,
// `~0xffffffffu` is the `uint` value 0, not the 64-bit value 0xffffffff00000000.
//
// Each case is a `static const int` that is 1 when the folded expression has the expected
// value and 0 otherwise. A `switch` then uses the constant as a second case label next to
// `case 0`, which makes the front end fold it to a concrete value. A wrong result folds to 0
// and fails with "duplicate cases in switch".
//
// The checks widen the literal to `uint64_t` before comparing it. A comparison in the literal's
// own type would narrow both operands first and hide high bits that the parser left behind.

static const int bitnot_uint = (uint64_t(~0xffffffffu) == 0ull) ? 1 : 0;
static const int neg_uint = (uint64_t(-1u) == 0xffffffffull) ? 1 : 0;
static const int neg_uint_top_bit = (uint64_t(-0x80000000u) == 0x80000000ull) ? 1 : 0;
static const int bitnot_int = (int64_t(~0x7fffffff) == int64_t(-2147483648)) ? 1 : 0;

// A literal initializer is folded when the declaration header is checked, before the
// initializer itself is checked, so it takes a separate path from the cases above.
static const uint64_t bitnot_uint_initializer = ~0xffffffffu;
static const int bitnot_uint_initializer_is_zero = (bitnot_uint_initializer == 0ull) ? 1 : 0;

void use()
{
    switch (0) { case 0: case bitnot_uint: break; }
    switch (0) { case 0: case neg_uint: break; }
    switch (0) { case 0: case neg_uint_top_bit: break; }
    switch (0) { case 0: case bitnot_int: break; }
    switch (0) { case 0: case bitnot_uint_initializer_is_zero: break; }
}
