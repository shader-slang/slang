//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Fold diagnostics for a `static const` are reported once, by the fold that first sees the
// operand values (see `SemanticsVisitor::getFoldDiagnosticSink`).
//
// For a non-generic constant, checking the declaration folds the initializer and reports. Every
// use folds the same initializer again through `tryConstantFoldDeclRef` without reporting, so a
// use reports only the follow-up error of not getting a constant. For a constant in a generic
// type, the use supplies the generic arguments, so the use reports.

// A constant used three times is reported once.
static const int8_t used_three_times = int8_t(5) / int8_t(0);
//CHECK:                                         ^ divide by zero
void use_three_times()
{
    switch (int8_t(0)) { case 0: case used_three_times: break; }
//CHECK:                              ^^^^^^^^^^^^^^^^ could not extract value from integer constant
    switch (int8_t(0)) { case 0: case used_three_times: break; }
//CHECK:                              ^^^^^^^^^^^^^^^^ could not extract value from integer constant
    switch (int8_t(0)) { case 0: case used_three_times: break; }
//CHECK:                              ^^^^^^^^^^^^^^^^ could not extract value from integer constant
}

// A target-defined result is reported once, too.
static const int8_t shifted_too_far = int8_t(1) << int8_t(9);
//CHECK:                                        ^^ target-defined value
//CHECK:                                        ^^ the value of expression int8_t(1) << int8_t(9) is target-defined.
void use_shifted_too_far()
{
    switch (int8_t(0)) { case 0: case shifted_too_far: break; }
//CHECK:                              ^^^^^^^^^^^^^^^ could not extract value from integer constant
    switch (int8_t(0)) { case 0: case shifted_too_far: break; }
//CHECK:                              ^^^^^^^^^^^^^^^ could not extract value from integer constant
}

// A use that needs an implicit conversion folds the initializer more than once, and is still not
// reported again.
static const int64_t converted = int64_t(5) % int64_t(0);
//CHECK:                                    ^ divide by zero
void use_converted()
{
    switch (0) { case 0: case converted: break; }
//CHECK:                      ^^^^^^^^^ implicit conversion not recommended
//CHECK:                      ^^^^^^^^^ implicit conversion from 'int64_t' to 'int' is not recommended
//CHECK:                      ^^^^^^^^^ could not extract value from integer constant
}

// A constant whose initializer uses another failing constant reports nothing of its own; the
// failing constant is reported once, at its own initializer.
static const int8_t failing = int8_t(5) / int8_t(0);
//CHECK:                                ^ divide by zero
static const int8_t depends_on_failing = failing + int8_t(1);
void use_depends_on_failing()
{
    switch (int8_t(0)) { case 0: case depends_on_failing: break; }
//CHECK:                              ^^^^^^^^^^^^^^^^^^ could not extract value from integer constant
}

// The declaration check of a generic constant cannot see `N`, so the use that supplies `N = 0`
// reports the division by zero, at the initializer. A second use of `Generic<0>.quotient` would
// report it again (see the TODO in `SemanticsVisitor::tryConstantFoldDeclRef`).
struct Generic<let N : int>
{
    static const int quotient = 10 / N;
//CHECK:                           ^ divide by zero
}
void use_generic()
{
    switch (0) { case 0: case Generic<0>.quotient: break; }
//CHECK:                                 ^^^^^^^^ could not extract value from integer constant
    switch (0) { case 0: case Generic<5>.quotient: break; }
}
