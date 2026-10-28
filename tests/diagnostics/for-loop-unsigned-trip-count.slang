//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Trip-count inference for a `for` loop orders and subtracts the loop bounds as values of the type
// the loop predicate compares in (shader-slang/slang#13985). A `[MaxIters(100)]` attribute exposes
// the inferred count through the "MaxIters exceeds actual iterations" warning.

RWStructuredBuffer<uint> outBuf;

void inferredLoopCounts()
{
    // A `uint64_t` bound above INT64_MAX is not read as a negative number, so neither loop
    // reports running zero times. These are the loops from the issue; inference reads only the
    // loop header, and the `break` only keeps the loops finite at run time.
    for (uint64_t i = 0; i < 0xFFFFFFFFFFFFFFFFull; i++)
    {
        if (outBuf[0] == 0)
            break;
    }
    for (uint64_t j = 5; j < 0x8000000000000000ull; j++)
    {
        if (outBuf[0] == 0)
            break;
    }

    // Bounds on both sides of INT64_MAX, counting up and counting down.
    [MaxIters(100)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 4 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint64_t i = 0x7FFFFFFFFFFFFFFEull; i < 0x8000000000000002ull; i++)
    {
    }
    [MaxIters(100)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 3 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint64_t i = 0x8000000000000001ull; i > 0x7FFFFFFFFFFFFFFEull; i -= 1)
    {
    }
    [MaxIters(100)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 1 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint64_t i = 0x8000000000000000ull; i <= 0x8000000000000000ull; i++)
    {
    }

    // The constant can be on the left of the predicate, and the induction variable can be
    // initialized by an assignment.
    [MaxIters(100)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 4 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint64_t i = 0x7FFFFFFFFFFFFFFEull; 0x8000000000000002ull > i; i++)
    {
    }
    uint64_t k;
    [MaxIters(100)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 2 iterations, which is less than what [MaxIters] specifies.
*/
    for (k = 0x8000000000000001ull; k >= 0x8000000000000000ull; k--)
    {
    }

    // `uintptr_t` (and so `size_t`) is ordered as unsigned 64-bit too.
    [MaxIters(100)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 4 iterations, which is less than what [MaxIters] specifies.
*/
    for (uintptr_t i = 0x7FFFFFFFFFFFFFFEuz; i < 0x8000000000000002uz; i++)
    {
    }

    // Signed bounds on both sides of zero keep their signed order.
    [MaxIters(100)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 5 iterations, which is less than what [MaxIters] specifies.
*/
    for (int64_t i = -3; i < 2; i++)
    {
    }

    // Loops that really run zero times still warn.
    for (uint64_t i = 0xFFFFFFFFFFFFFFFFull; i < 5; i++)
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/
    {
    }
    for (uint64_t i = 0x8000000000000000ull; i < 0x8000000000000000ull; i++)
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/
    {
    }
    // `-1` converts to 0xFFFFFFFF, so the predicate is false on entry.
    for (uint i = 1; i > -1; i--)
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/
    {
    }
    for (int64_t i = 0; i < -1; i++)
/*diag:
    ^^^ loop runs zero times
    ^^^ the loop runs for 0 iterations and will be removed.
*/
    {
    }

    // A count above INT32_MAX does not fit the inferred bound, so no bound is inferred and
    // `[MaxIters]` is not reported as exceeding it.
    [MaxIters(100)]
    for (int64_t i = 0; i < 0x100000000ll; i++)
    {
    }
    [MaxIters(100)]
    for (uint i = 0; i < 0xFFFFFFFFu; i++)
    {
    }
    // The distance between these signed bounds is above INT64_MAX.
    [MaxIters(100)]
    for (int64_t i = -0x7000000000000000ll; i < 0x7000000000000000ll; i++)
    {
    }
    // The full `uint64_t` range has 2^64 values, one more than a `uint64_t` count can hold.
    for (uint64_t i = 0; i <= 0xFFFFFFFFFFFFFFFFull; i++)
    {
        if (outBuf[0] == 0)
            break;
    }

    // A count of 2^31, one above INT32_MAX, infers no bound, while a count of 2^31 - 2 is still
    // inferred.
    [MaxIters(100)]
    for (int64_t i = 0; i < 0x80000000ll; i++)
    {
    }
    [MaxIters(2147483647)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 2147483646 iterations, which is less than what [MaxIters] specifies.
*/
    for (int64_t i = 1; i < 0x7FFFFFFFll; i++)
    {
    }

    // A negative step added to the variable counts down by its magnitude.
    [MaxIters(100)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 5 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 10; i > 0; i += -2)
    {
    }

    // An exclusive bound with a step that does not divide the distance rounds the count up.
    [MaxIters(100)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 4 iterations, which is less than what [MaxIters] specifies.
*/
    for (int i = 0; i < 10; i += 3)
    {
    }
    [MaxIters(100)]
/*diag:
     ^^^^^^^^ MaxIters exceeds actual iterations
     ^^^^^^^^ the for loop is statically determined to terminate within 5 iterations, which is less than what [MaxIters] specifies.
*/
    for (uint64_t i = 0x7FFFFFFFFFFFFFF0ull; i < 0x8000000000000010ull; i += 7)
    {
    }
}

// A differentiable loop whose count is above INT32_MAX gets no inferred bound, so it needs an
// explicit `[MaxIters]` or `[ForceUnroll]`.
[Differentiable]
float sumAboveCap(float x)
{
    float r = 0.0;
    for (int64_t i = 0; i < 0x100000000ll; i++)
/*diag:
    ^^^ loop in differentiable function needs attributes
    ^^^ loops inside a differentiable function need to provide either '[MaxIters(n)]' or '[ForceUnroll]' attribute.
*/
    {
        r += x;
    }
    return r;
}
