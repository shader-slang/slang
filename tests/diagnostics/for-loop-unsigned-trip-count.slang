//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Trip-count inference for a `for` loop orders and subtracts the loop bounds as values of the type
// the loop predicate compares in (shader-slang/slang#13985). A `[MaxIters(100)]` attribute exposes
// the inferred count through the "MaxIters exceeds actual iterations" warning.

RWStructuredBuffer<uint> outBuf;

void unsignedBounds()
{
    // A `uint64_t` bound above INT64_MAX is not read as a negative number, so neither loop
    // reports running zero times.
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
    [MaxIters(100)]
    for (int64_t i = -0x7000000000000000ll; i < 0x7000000000000000ll; i++)
    {
    }
    for (uint64_t i = 0; i <= 0xFFFFFFFFFFFFFFFFull; i++)
    {
        if (outBuf[0] == 0)
            break;
    }
}
