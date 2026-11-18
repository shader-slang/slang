//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Diagnostics from front-end constant folding of the division operators `/` and `%` on int8_t, uint8_t,
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
// The fold diagnostic is reported once, when the declaration is checked. The `case` label folds
// the same initializer again through `tryConstantFoldDeclRef`, which suppresses the diagnostic
// (see `SemanticsVisitor::getFoldDiagnosticSink`), so the label reports only the follow-up error.
//
// The test pins a current behavior that is not intended to be permanent: the follow-up error at
// the `case` label differs between the paths. The built-in path leaves a symbolic
// `BuiltinOperationIntVal` ("could not extract value from integer constant"), while the named
// path fails to fold ("expression does not evaluate to a compile-time constant").

// Division and remainder by zero are undefined.

static const int8_t div_i8_by_zero_b = int8_t(5) / int8_t(0);
//CHECK:                                         ^ error E30002
void use_div_i8_by_zero_b() { switch (int8_t(0)) { case 0: case div_i8_by_zero_b: break; } }
//CHECK:                                                        ^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int8_t div_i8_by_zero_n = operator/(int8_t(5), int8_t(0));
//CHECK:                               ^^^^^^^^ error E30002
void use_div_i8_by_zero_n() { switch (int8_t(0)) { case 0: case div_i8_by_zero_n: break; } }
//CHECK:                                                        ^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int8_t rem_i8_by_zero_b = int8_t(5) % int8_t(0);
//CHECK:                                         ^ error E30002
void use_rem_i8_by_zero_b() { switch (int8_t(0)) { case 0: case rem_i8_by_zero_b: break; } }
//CHECK:                                                        ^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int8_t rem_i8_by_zero_n = operator%(int8_t(5), int8_t(0));
//CHECK:                               ^^^^^^^^ error E30002
void use_rem_i8_by_zero_n() { switch (int8_t(0)) { case 0: case rem_i8_by_zero_n: break; } }
//CHECK:                                                        ^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint8_t div_u8_by_zero_b = uint8_t(0x05) / uint8_t(0x00);
//CHECK:                                              ^ error E30002
void use_div_u8_by_zero_b() { switch (uint8_t(0)) { case 0: case div_u8_by_zero_b: break; } }
//CHECK:                                                         ^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint8_t div_u8_by_zero_n = operator/(uint8_t(0x05), uint8_t(0x00));
//CHECK:                                ^^^^^^^^ error E30002
void use_div_u8_by_zero_n() { switch (uint8_t(0)) { case 0: case div_u8_by_zero_n: break; } }
//CHECK:                                                         ^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint8_t rem_u8_by_zero_b = uint8_t(0x05) % uint8_t(0x00);
//CHECK:                                              ^ error E30002
void use_rem_u8_by_zero_b() { switch (uint8_t(0)) { case 0: case rem_u8_by_zero_b: break; } }
//CHECK:                                                         ^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint8_t rem_u8_by_zero_n = operator%(uint8_t(0x05), uint8_t(0x00));
//CHECK:                                ^^^^^^^^ error E30002
void use_rem_u8_by_zero_n() { switch (uint8_t(0)) { case 0: case rem_u8_by_zero_n: break; } }
//CHECK:                                                         ^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int64_t div_i64_by_zero_b = int64_t(5) / int64_t(0);
//CHECK:                                            ^ error E30002
void use_div_i64_by_zero_b() { switch (int64_t(0)) { case 0: case div_i64_by_zero_b: break; } }
//CHECK:                                                          ^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int64_t div_i64_by_zero_n = operator/(int64_t(5), int64_t(0));
//CHECK:                                 ^^^^^^^^ error E30002
void use_div_i64_by_zero_n() { switch (int64_t(0)) { case 0: case div_i64_by_zero_n: break; } }
//CHECK:                                                          ^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int64_t rem_i64_by_zero_b = int64_t(5) % int64_t(0);
//CHECK:                                            ^ error E30002
void use_rem_i64_by_zero_b() { switch (int64_t(0)) { case 0: case rem_i64_by_zero_b: break; } }
//CHECK:                                                          ^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int64_t rem_i64_by_zero_n = operator%(int64_t(5), int64_t(0));
//CHECK:                                 ^^^^^^^^ error E30002
void use_rem_i64_by_zero_n() { switch (int64_t(0)) { case 0: case rem_i64_by_zero_n: break; } }
//CHECK:                                                          ^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint64_t div_u64_by_zero_b = uint64_t(5) / uint64_t(0);
//CHECK:                                              ^ error E30002
void use_div_u64_by_zero_b() { switch (uint64_t(0)) { case 0: case div_u64_by_zero_b: break; } }
//CHECK:                                                           ^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint64_t div_u64_by_zero_n = operator/(uint64_t(5), uint64_t(0));
//CHECK:                                  ^^^^^^^^ error E30002
void use_div_u64_by_zero_n() { switch (uint64_t(0)) { case 0: case div_u64_by_zero_n: break; } }
//CHECK:                                                           ^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const uint64_t rem_u64_by_zero_b = uint64_t(5) % uint64_t(0);
//CHECK:                                              ^ error E30002
void use_rem_u64_by_zero_b() { switch (uint64_t(0)) { case 0: case rem_u64_by_zero_b: break; } }
//CHECK:                                                           ^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const uint64_t rem_u64_by_zero_n = operator%(uint64_t(5), uint64_t(0));
//CHECK:                                  ^^^^^^^^ error E30002
void use_rem_u64_by_zero_n() { switch (uint64_t(0)) { case 0: case rem_u64_by_zero_n: break; } }
//CHECK:                                                           ^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

// Signed `MIN / -1` and `MIN % -1` overflow and are target-defined.

static const int8_t div_i8_min_by_minus_one_b = int8_t(-128) / int8_t(-1);
//CHECK:                                                     ^ target-defined value
//CHECK:                                                     ^ the value of expression int8_t(-128) / int8_t(-1) is target-defined.
void use_div_i8_min_by_minus_one_b() { switch (int8_t(0)) { case 0: case div_i8_min_by_minus_one_b: break; } }
//CHECK:                                                                 ^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int8_t div_i8_min_by_minus_one_n = operator/(int8_t(-128), int8_t(-1));
//CHECK:                                        ^^^^^^^^ target-defined value
//CHECK:                                        ^^^^^^^^ the value of expression int8_t(-128) / int8_t(-1) is target-defined.
void use_div_i8_min_by_minus_one_n() { switch (int8_t(0)) { case 0: case div_i8_min_by_minus_one_n: break; } }
//CHECK:                                                                 ^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int8_t rem_i8_min_by_minus_one_b = int8_t(-128) % int8_t(-1);
//CHECK:                                                     ^ target-defined value
//CHECK:                                                     ^ the value of expression int8_t(-128) % int8_t(-1) is target-defined.
void use_rem_i8_min_by_minus_one_b() { switch (int8_t(0)) { case 0: case rem_i8_min_by_minus_one_b: break; } }
//CHECK:                                                                 ^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int8_t rem_i8_min_by_minus_one_n = operator%(int8_t(-128), int8_t(-1));
//CHECK:                                        ^^^^^^^^ target-defined value
//CHECK:                                        ^^^^^^^^ the value of expression int8_t(-128) % int8_t(-1) is target-defined.
void use_rem_i8_min_by_minus_one_n() { switch (int8_t(0)) { case 0: case rem_i8_min_by_minus_one_n: break; } }
//CHECK:                                                                 ^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int64_t div_i64_min_by_minus_one_b = int64_t(0x8000000000000000ull) / int64_t(-1);
//CHECK:                                                                         ^ target-defined value
//CHECK:                                                                         ^ the value of expression -9223372036854775808LL / -1LL is target-defined.
void use_div_i64_min_by_minus_one_b() { switch (int64_t(0)) { case 0: case div_i64_min_by_minus_one_b: break; } }
//CHECK:                                                                   ^^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int64_t div_i64_min_by_minus_one_n = operator/(int64_t(0x8000000000000000ull), int64_t(-1));
//CHECK:                                          ^^^^^^^^ target-defined value
//CHECK:                                          ^^^^^^^^ the value of expression -9223372036854775808LL / -1LL is target-defined.
void use_div_i64_min_by_minus_one_n() { switch (int64_t(0)) { case 0: case div_i64_min_by_minus_one_n: break; } }
//CHECK:                                                                   ^^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant

static const int64_t rem_i64_min_by_minus_one_b = int64_t(0x8000000000000000ull) % int64_t(-1);
//CHECK:                                                                         ^ target-defined value
//CHECK:                                                                         ^ the value of expression -9223372036854775808LL % -1LL is target-defined.
void use_rem_i64_min_by_minus_one_b() { switch (int64_t(0)) { case 0: case rem_i64_min_by_minus_one_b: break; } }
//CHECK:                                                                   ^^^^^^^^^^^^^^^^^^^^^^^^^^ could not extract value from integer constant

static const int64_t rem_i64_min_by_minus_one_n = operator%(int64_t(0x8000000000000000ull), int64_t(-1));
//CHECK:                                          ^^^^^^^^ target-defined value
//CHECK:                                          ^^^^^^^^ the value of expression -9223372036854775808LL % -1LL is target-defined.
void use_rem_i64_min_by_minus_one_n() { switch (int64_t(0)) { case 0: case rem_i64_min_by_minus_one_n: break; } }
//CHECK:                                                                   ^^^^^^^^^^^^^^^^^^^^^^^^^^ expression does not evaluate to a compile-time constant
