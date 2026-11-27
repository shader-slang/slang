//DIAGNOSTIC_TEST:SIMPLE(diag=diag,exit-code=0):

// For-loop trip-count inference with operand types. For a loop of the form
// `for (T i = init; i OP bound; i STEP step)` with constant `init`, `bound` and `step`, the
// checker infers how many times the body runs. The comparison uses the common type of `i` and
// `bound`, and the step changes `i` modulo 2^width of `T`. Nothing is inferred when `i` would
// leave the range of `T` before the condition fails, because the loop then wraps around and
// may never end, or when the count does not fit in `int`.
//
// Each loop has `[MaxIters(2147483647)]`, so an inferred count N below that limit shows up as
// "statically determined to terminate within N iterations". A loop whose body never runs also
// reports "loop runs zero times". A loop with no inferred count reports neither.
// `tests/front-end/for-loop-trip-count-limits.slang` covers counts at the limit of `int`.
//
// The same inference also reports a step that moves the variable away from the bound and a
// condition that checks a different variable than the one the loop initializes and steps. Some
// cases check these warnings too.

void f(int j, uint n)
{
    // Unsigned 64-bit and 32-bit bounds with the top bit set. These compare as unsigned values.

    // A1: no inferred count. 2^64 - 1 iterations do not fit in `int`.
    [MaxIters(2147483647)]
    for (uint64_t i = 0; i < 0xFFFFFFFFFFFFFFFFull; i++) {}

    // A2: no inferred count. 2^63 - 5 iterations do not fit in `int`.
    [MaxIters(2147483647)]
    for (uint64_t i = 5; i < 0x8000000000000000ull; i++) {}

    // A3: 15 iterations.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 15 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint64_t i = 0xFFFFFFFFFFFFFFF0ull; i < 0xFFFFFFFFFFFFFFFFull; i++) {}

    // A4: 5 iterations.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 5 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint64_t i = 0x8000000000000002ull; i >= 0x7FFFFFFFFFFFFFFEull; i--) {}

    // A5: 15 iterations.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 15 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint i = 0xFFFFFFF0u; i < 0xFFFFFFFFu; i++) {}

    // A6: no inferred count. `i` wraps from 2^63 + 2^63 to 0.
    [MaxIters(2147483647)]
    for (uint64_t i = 0; i < 0xFFFFFFFFFFFFFFFFull; i += 0x8000000000000000ull) {}

    // A7: no inferred count. `i` wraps from 2^63 - 1 - 2^63 to 0xFFFFFFFFFFFFFFFF.
    [MaxIters(2147483647)]
    for (uint64_t i = 0xFFFFFFFFFFFFFFFFull; i > 0ull; i -= 0x8000000000000000ull) {}

    // A8: 1 iteration. The second value 2^63 fails the condition.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 1 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint64_t i = 0; i < 0x8000000000000000ull; i += 0x8000000000000000ull) {}

    // Loops whose variable leaves the range of its type before the condition fails. The variable
    // wraps around, so the loop may never end, and nothing is inferred.

    // B1: no inferred count. `i` wraps from 0xFFFFFFFF to 0.
    [MaxIters(2147483647)]
    for (uint i = 0xFFFFFFF0u; i <= 0xFFFFFFFFu; i++) {}

    // B2: no inferred count. `i` wraps from 255 to 0 and never reaches 300.
    [MaxIters(2147483647)]
    for (uint8_t i = 0; i < 300; i++) {}
//diag:                   ^ comparison value is not representable by the iterator
//diag:                   ^ the comparison value 300 is not representable by the iterator type 'uint8_t'.

    // B3: 5 iterations. The comparison is in `int`.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 5 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint8_t i = 250; i < 255; i++) {}

    // B4: no inferred count. `i` wraps from 255 to 0.
    [MaxIters(2147483647)]
    for (uint8_t i = 250; i <= 255; i++) {}

    // B5: no inferred count. `i` wraps from 127 to -128.
    [MaxIters(2147483647)]
    for (int8_t i = 120; i <= 127; i++) {}

    // B6: no inferred count. `i` wraps from 254 + 2 to 0.
    [MaxIters(2147483647)]
    for (uint8_t i = 0; i < 255; i += 2) {}

    // B7: 127 iterations.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 127 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint8_t i = 0; i < 254; i += 2) {}

    // Loops that count down.

    // C1: no inferred count. `i >= 0` always holds for `uint`; `i` wraps from 0 to 0xFFFFFFFF.
    [MaxIters(2147483647)]
    for (uint i = 5; i >= 0; i--) {}

    // C2: 5 iterations.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 5 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint i = 5; i > 0; i--) {}

    // C3: 4 iterations. 10, -20, -50, -80.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 4 iterations, which is less than what [MaxIters] specifies.
*/
    for (int8_t i = 10; i > -100; i -= 30) {}

    // C4: no inferred count. -110 - 30 wraps around.
    [MaxIters(2147483647)]
    for (int8_t i = 10; i > -128; i -= 30) {}

    // A signed loop variable compared with an unsigned bound. The comparison uses the common type
    // of the two operands.

    // D1: the body never runs. The comparison is in `uint`, where -5 is 0xFFFFFFFB.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 0 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = -5; i < 10u; i++) {}
//diag:                ^ initial value changes in comparison due to an implicit cast
//diag:                ^ the comparison casts the initial value to type 'uint', which changes its value from -5 to 4294967291U.
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/

    // D2: 10 iterations.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 10 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 0; i < 10u; i++) {}

    // D3: 15 iterations. The comparison is in `int64_t`.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 15 iterations, which is less than what [MaxIters] specifies.
*/
    for (int64_t i = -5; i < 10u; i++) {}

    // D4: no inferred count. The comparison is in `uint`, where `i` goes from 0 to 0xFFFFFFFF, and
    // `i >= 0u` always holds.
    [MaxIters(2147483647)]
    for (int i = 5; i >= 0u; i--) {}

    // D5: no inferred count. The comparison is in `uint`, where `i` goes from 0xFFFFFFFF to 0, and
    // `i <= 0xFFFFFFFFu` always holds.
    [MaxIters(2147483647)]
    for (int i = -5; i <= 0xFFFFFFFFu; i++) {}
//diag:                ^^ initial value changes in comparison due to an implicit cast
//diag:                ^^ the comparison casts the initial value to type 'uint', which changes its value from -5 to 4294967291U.
//diag:                ^^ comparison value is not representable by the iterator
//diag:                ^^ the comparison value 4294967295U is not representable by the iterator type 'int'.

    // D6: 5 iterations. `i` stops at 0 and does not wrap in `uint`.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 5 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 5; i > 0u; i--) {}

    // D7: no inferred count. The body runs 7 times, from 0xFFFFFFF6 down to 0xFFFFFFF0 in `uint`,
    // but the comparison changes the initial value, so nothing is inferred.
    [MaxIters(2147483647)]
    for (int i = -10; i >= 0xFFFFFFF0u; i--) {}
//diag:                 ^^ initial value changes in comparison due to an implicit cast
//diag:                 ^^ the comparison casts the initial value to type 'uint', which changes its value from -10 to 4294967286U.
//diag:                 ^^ comparison value is not representable by the iterator
//diag:                 ^^ the comparison value 4294967280U is not representable by the iterator type 'int'.

    // Steps whose type or sign differs from the loop variable, and steps that move the variable
    // away from the bound.

    // E2: 5 iterations.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 5 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 0; i < 10; i -= -2) {}

    // E3: wrong direction, no inferred count. `i--` moves `i` away from the bound of `<=`.
    [MaxIters(2147483647)]
    for (int i = 0; i <= 5; i--) {}
//diag:                      ^^ for loop modifies variable in wrong direction
//diag:                      ^^ the for loop is modifiying variable 'i' in the opposite direction from loop exit condition.

    // E4: wrong direction, no inferred count. `i++` moves `i` away from the bound of `>=`.
    [MaxIters(2147483647)]
    for (int i = 5; i >= 0; i++) {}
//diag:                      ^^ for loop modifies variable in wrong direction
//diag:                      ^^ the for loop is modifiying variable 'i' in the opposite direction from loop exit condition.

    // E5: wrong direction, no inferred count. `i += -1` moves `i` away from the bound of `<=`.
    [MaxIters(2147483647)]
    for (int i = 0; i <= 5; i += -1) {}
//diag:                       ^^ for loop modifies variable in wrong direction
//diag:                       ^^ the for loop is modifiying variable 'i' in the opposite direction from loop exit condition.

    // E6: wrong direction, no inferred count. `i -= -1` moves `i` away from the bound of `>=`.
    [MaxIters(2147483647)]
    for (int i = 5; i >= 0; i -= -1) {}
//diag:                       ^^ for loop modifies variable in wrong direction
//diag:                       ^^ the for loop is modifiying variable 'i' in the opposite direction from loop exit condition.

    // E7: wrong direction, no inferred count. `i--` moves an unsigned `i` away from the bound of `<=`.
    [MaxIters(2147483647)]
    for (uint i = 0; i <= 5u; i--) {}
//diag:                        ^^ for loop modifies variable in wrong direction
//diag:                        ^^ the for loop is modifiying variable 'i' in the opposite direction from loop exit condition.

    // Zero and one iteration.

    // G1: the body never runs.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 0 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint i = 0; i < 0u; i++) {}
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/

    // G2: 1 iteration.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 1 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint i = 0; i <= 0u; i++) {}

    // G3: the body never runs.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 0 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint i = 10; i < 9u; i++) {}
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/

    // G4: 1 iteration.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 1 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint i = 0; i < 1u; i++) {}

    // G5: the body never runs.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 0 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint i = 0; i > 0u; i--) {}
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/

    // G6: 1 iteration.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 1 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint i = 1; i > 0u; i--) {}

    // G7: 1 iteration.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 1 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 0; i >= 0; i--) {}

    // G8: no inferred count. `i` wraps from 0 to 0xFFFFFFFF after the first iteration.
    [MaxIters(2147483647)]
    for (uint i = 0; i >= 0u; i--) {}

    // G9: the body never runs.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 0 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint i = 0xFFFFFFFFu; i < 0xFFFFFFFFu; i++) {}
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/

    // G10: no inferred count. `i` wraps to 0 after the first iteration.
    [MaxIters(2147483647)]
    for (uint64_t i = 0xFFFFFFFFFFFFFFFFull; i <= 0xFFFFFFFFFFFFFFFFull; i++) {}

    // G11: 1 iteration. The comparison is in `int`.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 1 iterations, which is less than what [MaxIters] specifies.
*/
    for (int8_t i = -128; i < -127; i++) {}

    // G12: no inferred count. `i` wraps to 127 after the first iteration.
    [MaxIters(2147483647)]
    for (int8_t i = -128; i >= -128; i--) {}

    // G13: the body never runs. The condition is false before the step runs, so the step moving `i` away from the bound does not matter.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 0 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 1; i <= 0; i--) {}
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/

    // G14: the body never runs. The same with an exclusive bound.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 0 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 1; i < 0; i--) {}
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/

    // G15: the body never runs. The condition is false before the step runs, so the zero step does not matter.
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 0 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 1; i <= 0; i += 0) {}
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/

    // A condition that checks a different variable than the loop initializes and steps. Only the
    // wrong-variable warning is reported, and nothing is inferred.

    // H1: The cast of the initial value is not reported, because the condition does not check `i`.
    [MaxIters(2147483647)]
    for (int i = -1; j < 10u; i++) {}
//diag:                ^ for loop predicate checks wrong variable
//diag:                ^ the for loop initializes and modifies variable 'i' but the predicate expression is checking 'j'.

    // H2: The range of `uint8_t` is not reported, because the condition does not check `i`.
    [MaxIters(2147483647)]
    for (uint8_t i = 0; j < 300; i++) {}
//diag:                   ^ for loop predicate checks wrong variable
//diag:                   ^ the for loop initializes and modifies variable 'i' but the predicate expression is checking 'j'.

    // H3: `j` is converted to `uint` in the comparison.
    [MaxIters(2147483647)]
    for (int i = 0; j < n; i++) {}
//diag:               ^ for loop predicate checks wrong variable
//diag:               ^ the for loop initializes and modifies variable 'i' but the predicate expression is checking 'j'.

    // A condition that reads the loop variable inside a larger expression. The condition does
    // check `i`, so no wrong-variable warning is reported. The bound is not a constant, so nothing
    // is inferred.

    // H4: `j` is converted to `uint` in the comparison.
    [MaxIters(2147483647)]
    for (uint i = 0; j > i + 1u; ++i) {}

    // H5: both operands are `int`.
    [MaxIters(2147483647)]
    for (int i = 0; j > i + 1; ++i) {}
}
