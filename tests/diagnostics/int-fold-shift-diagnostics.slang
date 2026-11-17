//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Diagnostics from front-end constant folding of the shift operators `<<` and `>>` on int8_t, uint8_t,
// int64_t and uint64_t operands.
//
// Each case is a `static const` whose initializer cannot be folded, followed by a function that
// uses it as a `case` label. The label makes the front end fold the initializer to a concrete
// value, which fails after the fold diagnostic. The `switch` is on a value of the constant's own
// type so that the label needs no implicit conversion.
//
// The `_b` cases spell the operator infix, which the checker rewrites to a `BuiltinOperatorExpr`
// folded by `tryConstantFoldBuiltinOperatorExpr`. The `_n` cases call `operator OP(...)`
// explicitly, which stays an `InvokeExpr` folded by the operator table in
// `SemanticsVisitor::tryConstantFoldExpr`. Both paths end in `BuiltinOperationIntVal::tryFoldImpl`.
//
// The test pins two current behaviors that are not intended to be permanent:
//
// - The fold diagnostic is reported twice, once when the declaration is checked and once when
//   `tryConstantFoldDeclRef` folds the initializer again for the `case` label (see the TODO in
//   `BuiltinOperationIntVal::tryFoldImpl`).
// - The follow-up error at the `case` label differs between the paths: the built-in path leaves
//   a symbolic `BuiltinOperationIntVal` ("could not extract value from integer constant"), while
//   the named path fails to fold ("expression does not evaluate to a compile-time constant").

// Shift counts at or above the operand width, and negative shift counts, are
// target-defined. Shift counts have the same type as the shifted value.

static const int8_t lsh_i8_by_width_b = int8_t(1) << int8_t(8);
//CHECK:                                          ^^ target-defined value
//CHECK:                                          ^^ the value of expression int8_t(1) << int8_t(8) is target-defined.
//CHECK:                                          ^^ target-defined value
//CHECK:                                          ^^ the value of expression int8_t(1) << int8_t(8) is target-defined.
void use_lsh_i8_by_width_b() { switch (int8_t(0)) { case 0: case lsh_i8_by_width_b: break; } }
//CHECK:                                                         ^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int8_t lsh_i8_by_width_n = operator<<(int8_t(1), int8_t(8));
//CHECK:                                ^^^^^^^^ target-defined value
//CHECK:                                ^^^^^^^^ the value of expression int8_t(1) << int8_t(8) is target-defined.
//CHECK:                                ^^^^^^^^ target-defined value
//CHECK:                                ^^^^^^^^ the value of expression int8_t(1) << int8_t(8) is target-defined.
void use_lsh_i8_by_width_n() { switch (int8_t(0)) { case 0: case lsh_i8_by_width_n: break; } }
//CHECK:                                                         ^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int8_t lsh_i8_by_width_plus_one_b = int8_t(1) << int8_t(9);
//CHECK:                                                   ^^ target-defined value
//CHECK:                                                   ^^ the value of expression int8_t(1) << int8_t(9) is target-defined.
//CHECK:                                                   ^^ target-defined value
//CHECK:                                                   ^^ the value of expression int8_t(1) << int8_t(9) is target-defined.
void use_lsh_i8_by_width_plus_one_b() { switch (int8_t(0)) { case 0: case lsh_i8_by_width_plus_one_b: break; } }
//CHECK:                                                                  ^^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int8_t lsh_i8_by_width_plus_one_n = operator<<(int8_t(1), int8_t(9));
//CHECK:                                         ^^^^^^^^ target-defined value
//CHECK:                                         ^^^^^^^^ the value of expression int8_t(1) << int8_t(9) is target-defined.
//CHECK:                                         ^^^^^^^^ target-defined value
//CHECK:                                         ^^^^^^^^ the value of expression int8_t(1) << int8_t(9) is target-defined.
void use_lsh_i8_by_width_plus_one_n() { switch (int8_t(0)) { case 0: case lsh_i8_by_width_plus_one_n: break; } }
//CHECK:                                                                  ^^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int8_t lsh_i8_by_minus_one_b = int8_t(1) << int8_t(-1);
//CHECK:                                              ^^ target-defined value
//CHECK:                                              ^^ the value of expression int8_t(1) << int8_t(-1) is target-defined.
//CHECK:                                              ^^ target-defined value
//CHECK:                                              ^^ the value of expression int8_t(1) << int8_t(-1) is target-defined.
void use_lsh_i8_by_minus_one_b() { switch (int8_t(0)) { case 0: case lsh_i8_by_minus_one_b: break; } }
//CHECK:                                                             ^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int8_t lsh_i8_by_minus_one_n = operator<<(int8_t(1), int8_t(-1));
//CHECK:                                    ^^^^^^^^ target-defined value
//CHECK:                                    ^^^^^^^^ the value of expression int8_t(1) << int8_t(-1) is target-defined.
//CHECK:                                    ^^^^^^^^ target-defined value
//CHECK:                                    ^^^^^^^^ the value of expression int8_t(1) << int8_t(-1) is target-defined.
void use_lsh_i8_by_minus_one_n() { switch (int8_t(0)) { case 0: case lsh_i8_by_minus_one_n: break; } }
//CHECK:                                                             ^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int8_t rsh_i8_by_width_b = int8_t(1) >> int8_t(8);
//CHECK:                                          ^^ target-defined value
//CHECK:                                          ^^ the value of expression int8_t(1) >> int8_t(8) is target-defined.
//CHECK:                                          ^^ target-defined value
//CHECK:                                          ^^ the value of expression int8_t(1) >> int8_t(8) is target-defined.
void use_rsh_i8_by_width_b() { switch (int8_t(0)) { case 0: case rsh_i8_by_width_b: break; } }
//CHECK:                                                         ^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int8_t rsh_i8_by_width_n = operator>>(int8_t(1), int8_t(8));
//CHECK:                                ^^^^^^^^ target-defined value
//CHECK:                                ^^^^^^^^ the value of expression int8_t(1) >> int8_t(8) is target-defined.
//CHECK:                                ^^^^^^^^ target-defined value
//CHECK:                                ^^^^^^^^ the value of expression int8_t(1) >> int8_t(8) is target-defined.
void use_rsh_i8_by_width_n() { switch (int8_t(0)) { case 0: case rsh_i8_by_width_n: break; } }
//CHECK:                                                         ^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int8_t rsh_i8_by_width_plus_one_b = int8_t(1) >> int8_t(9);
//CHECK:                                                   ^^ target-defined value
//CHECK:                                                   ^^ the value of expression int8_t(1) >> int8_t(9) is target-defined.
//CHECK:                                                   ^^ target-defined value
//CHECK:                                                   ^^ the value of expression int8_t(1) >> int8_t(9) is target-defined.
void use_rsh_i8_by_width_plus_one_b() { switch (int8_t(0)) { case 0: case rsh_i8_by_width_plus_one_b: break; } }
//CHECK:                                                                  ^^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int8_t rsh_i8_by_width_plus_one_n = operator>>(int8_t(1), int8_t(9));
//CHECK:                                         ^^^^^^^^ target-defined value
//CHECK:                                         ^^^^^^^^ the value of expression int8_t(1) >> int8_t(9) is target-defined.
//CHECK:                                         ^^^^^^^^ target-defined value
//CHECK:                                         ^^^^^^^^ the value of expression int8_t(1) >> int8_t(9) is target-defined.
void use_rsh_i8_by_width_plus_one_n() { switch (int8_t(0)) { case 0: case rsh_i8_by_width_plus_one_n: break; } }
//CHECK:                                                                  ^^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int8_t rsh_i8_by_minus_one_b = int8_t(1) >> int8_t(-1);
//CHECK:                                              ^^ target-defined value
//CHECK:                                              ^^ the value of expression int8_t(1) >> int8_t(-1) is target-defined.
//CHECK:                                              ^^ target-defined value
//CHECK:                                              ^^ the value of expression int8_t(1) >> int8_t(-1) is target-defined.
void use_rsh_i8_by_minus_one_b() { switch (int8_t(0)) { case 0: case rsh_i8_by_minus_one_b: break; } }
//CHECK:                                                             ^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int8_t rsh_i8_by_minus_one_n = operator>>(int8_t(1), int8_t(-1));
//CHECK:                                    ^^^^^^^^ target-defined value
//CHECK:                                    ^^^^^^^^ the value of expression int8_t(1) >> int8_t(-1) is target-defined.
//CHECK:                                    ^^^^^^^^ target-defined value
//CHECK:                                    ^^^^^^^^ the value of expression int8_t(1) >> int8_t(-1) is target-defined.
void use_rsh_i8_by_minus_one_n() { switch (int8_t(0)) { case 0: case rsh_i8_by_minus_one_n: break; } }
//CHECK:                                                             ^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint8_t lsh_u8_by_width_b = uint8_t(0x01) << uint8_t(0x08);
//CHECK:                                               ^^ target-defined value
//CHECK:                                               ^^ the value of expression uint8_t(1) << uint8_t(8) is target-defined.
//CHECK:                                               ^^ target-defined value
//CHECK:                                               ^^ the value of expression uint8_t(1) << uint8_t(8) is target-defined.
void use_lsh_u8_by_width_b() { switch (uint8_t(0)) { case 0: case lsh_u8_by_width_b: break; } }
//CHECK:                                                          ^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint8_t lsh_u8_by_width_n = operator<<(uint8_t(0x01), uint8_t(0x08));
//CHECK:                                 ^^^^^^^^ target-defined value
//CHECK:                                 ^^^^^^^^ the value of expression uint8_t(1) << uint8_t(8) is target-defined.
//CHECK:                                 ^^^^^^^^ target-defined value
//CHECK:                                 ^^^^^^^^ the value of expression uint8_t(1) << uint8_t(8) is target-defined.
void use_lsh_u8_by_width_n() { switch (uint8_t(0)) { case 0: case lsh_u8_by_width_n: break; } }
//CHECK:                                                          ^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint8_t lsh_u8_by_width_plus_one_b = uint8_t(0x01) << uint8_t(0x09);
//CHECK:                                                        ^^ target-defined value
//CHECK:                                                        ^^ the value of expression uint8_t(1) << uint8_t(9) is target-defined.
//CHECK:                                                        ^^ target-defined value
//CHECK:                                                        ^^ the value of expression uint8_t(1) << uint8_t(9) is target-defined.
void use_lsh_u8_by_width_plus_one_b() { switch (uint8_t(0)) { case 0: case lsh_u8_by_width_plus_one_b: break; } }
//CHECK:                                                                   ^^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint8_t lsh_u8_by_width_plus_one_n = operator<<(uint8_t(0x01), uint8_t(0x09));
//CHECK:                                          ^^^^^^^^ target-defined value
//CHECK:                                          ^^^^^^^^ the value of expression uint8_t(1) << uint8_t(9) is target-defined.
//CHECK:                                          ^^^^^^^^ target-defined value
//CHECK:                                          ^^^^^^^^ the value of expression uint8_t(1) << uint8_t(9) is target-defined.
void use_lsh_u8_by_width_plus_one_n() { switch (uint8_t(0)) { case 0: case lsh_u8_by_width_plus_one_n: break; } }
//CHECK:                                                                   ^^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint8_t rsh_u8_by_width_b = uint8_t(0x01) >> uint8_t(0x08);
//CHECK:                                               ^^ target-defined value
//CHECK:                                               ^^ the value of expression uint8_t(1) >> uint8_t(8) is target-defined.
//CHECK:                                               ^^ target-defined value
//CHECK:                                               ^^ the value of expression uint8_t(1) >> uint8_t(8) is target-defined.
void use_rsh_u8_by_width_b() { switch (uint8_t(0)) { case 0: case rsh_u8_by_width_b: break; } }
//CHECK:                                                          ^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint8_t rsh_u8_by_width_n = operator>>(uint8_t(0x01), uint8_t(0x08));
//CHECK:                                 ^^^^^^^^ target-defined value
//CHECK:                                 ^^^^^^^^ the value of expression uint8_t(1) >> uint8_t(8) is target-defined.
//CHECK:                                 ^^^^^^^^ target-defined value
//CHECK:                                 ^^^^^^^^ the value of expression uint8_t(1) >> uint8_t(8) is target-defined.
void use_rsh_u8_by_width_n() { switch (uint8_t(0)) { case 0: case rsh_u8_by_width_n: break; } }
//CHECK:                                                          ^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint8_t rsh_u8_by_width_plus_one_b = uint8_t(0x01) >> uint8_t(0x09);
//CHECK:                                                        ^^ target-defined value
//CHECK:                                                        ^^ the value of expression uint8_t(1) >> uint8_t(9) is target-defined.
//CHECK:                                                        ^^ target-defined value
//CHECK:                                                        ^^ the value of expression uint8_t(1) >> uint8_t(9) is target-defined.
void use_rsh_u8_by_width_plus_one_b() { switch (uint8_t(0)) { case 0: case rsh_u8_by_width_plus_one_b: break; } }
//CHECK:                                                                   ^^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint8_t rsh_u8_by_width_plus_one_n = operator>>(uint8_t(0x01), uint8_t(0x09));
//CHECK:                                          ^^^^^^^^ target-defined value
//CHECK:                                          ^^^^^^^^ the value of expression uint8_t(1) >> uint8_t(9) is target-defined.
//CHECK:                                          ^^^^^^^^ target-defined value
//CHECK:                                          ^^^^^^^^ the value of expression uint8_t(1) >> uint8_t(9) is target-defined.
void use_rsh_u8_by_width_plus_one_n() { switch (uint8_t(0)) { case 0: case rsh_u8_by_width_plus_one_n: break; } }
//CHECK:                                                                   ^^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int64_t lsh_i64_by_width_b = int64_t(1) << int64_t(64);
//CHECK:                                             ^^ target-defined value
//CHECK:                                             ^^ the value of expression 1LL << 64LL is target-defined.
//CHECK:                                             ^^ target-defined value
//CHECK:                                             ^^ the value of expression 1LL << 64LL is target-defined.
void use_lsh_i64_by_width_b() { switch (int64_t(0)) { case 0: case lsh_i64_by_width_b: break; } }
//CHECK:                                                           ^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int64_t lsh_i64_by_width_n = operator<<(int64_t(1), int64_t(64));
//CHECK:                                  ^^^^^^^^ target-defined value
//CHECK:                                  ^^^^^^^^ the value of expression 1LL << 64LL is target-defined.
//CHECK:                                  ^^^^^^^^ target-defined value
//CHECK:                                  ^^^^^^^^ the value of expression 1LL << 64LL is target-defined.
void use_lsh_i64_by_width_n() { switch (int64_t(0)) { case 0: case lsh_i64_by_width_n: break; } }
//CHECK:                                                           ^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int64_t lsh_i64_by_width_plus_one_b = int64_t(1) << int64_t(65);
//CHECK:                                                      ^^ target-defined value
//CHECK:                                                      ^^ the value of expression 1LL << 65LL is target-defined.
//CHECK:                                                      ^^ target-defined value
//CHECK:                                                      ^^ the value of expression 1LL << 65LL is target-defined.
void use_lsh_i64_by_width_plus_one_b() { switch (int64_t(0)) { case 0: case lsh_i64_by_width_plus_one_b: break; } }
//CHECK:                                                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int64_t lsh_i64_by_width_plus_one_n = operator<<(int64_t(1), int64_t(65));
//CHECK:                                           ^^^^^^^^ target-defined value
//CHECK:                                           ^^^^^^^^ the value of expression 1LL << 65LL is target-defined.
//CHECK:                                           ^^^^^^^^ target-defined value
//CHECK:                                           ^^^^^^^^ the value of expression 1LL << 65LL is target-defined.
void use_lsh_i64_by_width_plus_one_n() { switch (int64_t(0)) { case 0: case lsh_i64_by_width_plus_one_n: break; } }
//CHECK:                                                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int64_t lsh_i64_by_minus_one_b = int64_t(1) << int64_t(-1);
//CHECK:                                                 ^^ target-defined value
//CHECK:                                                 ^^ the value of expression 1LL << -1LL is target-defined.
//CHECK:                                                 ^^ target-defined value
//CHECK:                                                 ^^ the value of expression 1LL << -1LL is target-defined.
void use_lsh_i64_by_minus_one_b() { switch (int64_t(0)) { case 0: case lsh_i64_by_minus_one_b: break; } }
//CHECK:                                                               ^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int64_t lsh_i64_by_minus_one_n = operator<<(int64_t(1), int64_t(-1));
//CHECK:                                      ^^^^^^^^ target-defined value
//CHECK:                                      ^^^^^^^^ the value of expression 1LL << -1LL is target-defined.
//CHECK:                                      ^^^^^^^^ target-defined value
//CHECK:                                      ^^^^^^^^ the value of expression 1LL << -1LL is target-defined.
void use_lsh_i64_by_minus_one_n() { switch (int64_t(0)) { case 0: case lsh_i64_by_minus_one_n: break; } }
//CHECK:                                                               ^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int64_t rsh_i64_by_width_b = int64_t(1) >> int64_t(64);
//CHECK:                                             ^^ target-defined value
//CHECK:                                             ^^ the value of expression 1LL >> 64LL is target-defined.
//CHECK:                                             ^^ target-defined value
//CHECK:                                             ^^ the value of expression 1LL >> 64LL is target-defined.
void use_rsh_i64_by_width_b() { switch (int64_t(0)) { case 0: case rsh_i64_by_width_b: break; } }
//CHECK:                                                           ^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int64_t rsh_i64_by_width_n = operator>>(int64_t(1), int64_t(64));
//CHECK:                                  ^^^^^^^^ target-defined value
//CHECK:                                  ^^^^^^^^ the value of expression 1LL >> 64LL is target-defined.
//CHECK:                                  ^^^^^^^^ target-defined value
//CHECK:                                  ^^^^^^^^ the value of expression 1LL >> 64LL is target-defined.
void use_rsh_i64_by_width_n() { switch (int64_t(0)) { case 0: case rsh_i64_by_width_n: break; } }
//CHECK:                                                           ^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int64_t rsh_i64_by_width_plus_one_b = int64_t(1) >> int64_t(65);
//CHECK:                                                      ^^ target-defined value
//CHECK:                                                      ^^ the value of expression 1LL >> 65LL is target-defined.
//CHECK:                                                      ^^ target-defined value
//CHECK:                                                      ^^ the value of expression 1LL >> 65LL is target-defined.
void use_rsh_i64_by_width_plus_one_b() { switch (int64_t(0)) { case 0: case rsh_i64_by_width_plus_one_b: break; } }
//CHECK:                                                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int64_t rsh_i64_by_width_plus_one_n = operator>>(int64_t(1), int64_t(65));
//CHECK:                                           ^^^^^^^^ target-defined value
//CHECK:                                           ^^^^^^^^ the value of expression 1LL >> 65LL is target-defined.
//CHECK:                                           ^^^^^^^^ target-defined value
//CHECK:                                           ^^^^^^^^ the value of expression 1LL >> 65LL is target-defined.
void use_rsh_i64_by_width_plus_one_n() { switch (int64_t(0)) { case 0: case rsh_i64_by_width_plus_one_n: break; } }
//CHECK:                                                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int64_t rsh_i64_by_minus_one_b = int64_t(1) >> int64_t(-1);
//CHECK:                                                 ^^ target-defined value
//CHECK:                                                 ^^ the value of expression 1LL >> -1LL is target-defined.
//CHECK:                                                 ^^ target-defined value
//CHECK:                                                 ^^ the value of expression 1LL >> -1LL is target-defined.
void use_rsh_i64_by_minus_one_b() { switch (int64_t(0)) { case 0: case rsh_i64_by_minus_one_b: break; } }
//CHECK:                                                               ^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int64_t rsh_i64_by_minus_one_n = operator>>(int64_t(1), int64_t(-1));
//CHECK:                                      ^^^^^^^^ target-defined value
//CHECK:                                      ^^^^^^^^ the value of expression 1LL >> -1LL is target-defined.
//CHECK:                                      ^^^^^^^^ target-defined value
//CHECK:                                      ^^^^^^^^ the value of expression 1LL >> -1LL is target-defined.
void use_rsh_i64_by_minus_one_n() { switch (int64_t(0)) { case 0: case rsh_i64_by_minus_one_n: break; } }
//CHECK:                                                               ^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint64_t lsh_u64_by_width_b = uint64_t(1) << uint64_t(64);
//CHECK:                                               ^^ target-defined value
//CHECK:                                               ^^ the value of expression 1ULL << 64ULL is target-defined.
//CHECK:                                               ^^ target-defined value
//CHECK:                                               ^^ the value of expression 1ULL << 64ULL is target-defined.
void use_lsh_u64_by_width_b() { switch (uint64_t(0)) { case 0: case lsh_u64_by_width_b: break; } }
//CHECK:                                                            ^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint64_t lsh_u64_by_width_n = operator<<(uint64_t(1), uint64_t(64));
//CHECK:                                   ^^^^^^^^ target-defined value
//CHECK:                                   ^^^^^^^^ the value of expression 1ULL << 64ULL is target-defined.
//CHECK:                                   ^^^^^^^^ target-defined value
//CHECK:                                   ^^^^^^^^ the value of expression 1ULL << 64ULL is target-defined.
void use_lsh_u64_by_width_n() { switch (uint64_t(0)) { case 0: case lsh_u64_by_width_n: break; } }
//CHECK:                                                            ^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint64_t lsh_u64_by_width_plus_one_b = uint64_t(1) << uint64_t(65);
//CHECK:                                                        ^^ target-defined value
//CHECK:                                                        ^^ the value of expression 1ULL << 65ULL is target-defined.
//CHECK:                                                        ^^ target-defined value
//CHECK:                                                        ^^ the value of expression 1ULL << 65ULL is target-defined.
void use_lsh_u64_by_width_plus_one_b() { switch (uint64_t(0)) { case 0: case lsh_u64_by_width_plus_one_b: break; } }
//CHECK:                                                                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint64_t lsh_u64_by_width_plus_one_n = operator<<(uint64_t(1), uint64_t(65));
//CHECK:                                            ^^^^^^^^ target-defined value
//CHECK:                                            ^^^^^^^^ the value of expression 1ULL << 65ULL is target-defined.
//CHECK:                                            ^^^^^^^^ target-defined value
//CHECK:                                            ^^^^^^^^ the value of expression 1ULL << 65ULL is target-defined.
void use_lsh_u64_by_width_plus_one_n() { switch (uint64_t(0)) { case 0: case lsh_u64_by_width_plus_one_n: break; } }
//CHECK:                                                                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint64_t rsh_u64_by_width_b = uint64_t(1) >> uint64_t(64);
//CHECK:                                               ^^ target-defined value
//CHECK:                                               ^^ the value of expression 1ULL >> 64ULL is target-defined.
//CHECK:                                               ^^ target-defined value
//CHECK:                                               ^^ the value of expression 1ULL >> 64ULL is target-defined.
void use_rsh_u64_by_width_b() { switch (uint64_t(0)) { case 0: case rsh_u64_by_width_b: break; } }
//CHECK:                                                            ^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint64_t rsh_u64_by_width_n = operator>>(uint64_t(1), uint64_t(64));
//CHECK:                                   ^^^^^^^^ target-defined value
//CHECK:                                   ^^^^^^^^ the value of expression 1ULL >> 64ULL is target-defined.
//CHECK:                                   ^^^^^^^^ target-defined value
//CHECK:                                   ^^^^^^^^ the value of expression 1ULL >> 64ULL is target-defined.
void use_rsh_u64_by_width_n() { switch (uint64_t(0)) { case 0: case rsh_u64_by_width_n: break; } }
//CHECK:                                                            ^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint64_t rsh_u64_by_width_plus_one_b = uint64_t(1) >> uint64_t(65);
//CHECK:                                                        ^^ target-defined value
//CHECK:                                                        ^^ the value of expression 1ULL >> 65ULL is target-defined.
//CHECK:                                                        ^^ target-defined value
//CHECK:                                                        ^^ the value of expression 1ULL >> 65ULL is target-defined.
void use_rsh_u64_by_width_plus_one_b() { switch (uint64_t(0)) { case 0: case rsh_u64_by_width_plus_one_b: break; } }
//CHECK:                                                                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint64_t rsh_u64_by_width_plus_one_n = operator>>(uint64_t(1), uint64_t(65));
//CHECK:                                            ^^^^^^^^ target-defined value
//CHECK:                                            ^^^^^^^^ the value of expression 1ULL >> 65ULL is target-defined.
//CHECK:                                            ^^^^^^^^ target-defined value
//CHECK:                                            ^^^^^^^^ the value of expression 1ULL >> 65ULL is target-defined.
void use_rsh_u64_by_width_plus_one_n() { switch (uint64_t(0)) { case 0: case rsh_u64_by_width_plus_one_n: break; } }
//CHECK:                                                                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant
