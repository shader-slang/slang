// SPIR-V Unit Test Example
//
// Each compute entry point whose name starts with "test_" is an independent
// test case.  The CMake build compiles this file to SPIR-V with slangc, then
// CTest discovers and runs each test via the spvdb CLI:
//
//   spvdb test_shader.spv --entry test_arithmetic --run
//
// The CLI exits with code 0 on normal completion and code 1 on a panic
// (StopReason::Panic from the spvdb interpreter).

//-----------------------------------------------------------------------------
// Minimal assertion framework
//
// ASSERT_EQ(a, b) computes diff = a - b and calls _assert_fail(diff).
// _assert_fail reads from a 1-element groupshared array at index diff:
//   diff == 0  →  valid access  →  no panic  →  assertion passed
//   diff != 0  →  out-of-bounds  →  spvdb panics  →  assertion failed
//
// The groupshared write makes the read from _sentinel live (preventing the
// compiler from eliminating it as a dead store to a function-local variable).
//-----------------------------------------------------------------------------

groupshared int _assert_sink;

[noinline]
void _assert_fail(int diff)
{
    int _sentinel[1] = { 0 };
    // Reading _sentinel[diff] and writing to groupshared prevents dead-code
    // elimination.  When diff != 0 the access is out of bounds and spvdb stops
    // execution with StopReason::Panic.
    _assert_sink = _sentinel[diff];
}

void ASSERT_EQ(int a, int b)
{
    _assert_fail(a - b);  // diff 0 → pass, diff != 0 → OOB panic
}

void ASSERT_TRUE(bool cond)
{
    _assert_fail(int(!cond));  // true → 0 (pass), false → 1 (OOB panic)
}

//-----------------------------------------------------------------------------
// Test cases
//-----------------------------------------------------------------------------

// A passing test: basic integer arithmetic.
[shader("compute")]
[numthreads(1, 1, 1)]
void test_arithmetic()
{
    ASSERT_EQ(2 + 3,  5);
    ASSERT_EQ(10 - 4, 6);
    ASSERT_EQ(3 * 7,  21);
    ASSERT_EQ(20 / 4, 5);
}

// A passing test: bitwise operations.
[shader("compute")]
[numthreads(1, 1, 1)]
void test_bitwise()
{
    ASSERT_EQ(0xF0 & 0xFF, 0xF0);
    ASSERT_EQ(0x0F | 0xF0, 0xFF);
    ASSERT_EQ(0xFF ^ 0x0F, 0xF0);
    ASSERT_EQ(1 << 4,      16);
    ASSERT_TRUE((0xAB & 0x0F) == 0x0B);
}

// A FAILING test: contains a deliberate bug to demonstrate test failure.
[shader("compute")]
[numthreads(1, 1, 1)]
void test_bad_assumption()
{
    ASSERT_EQ(6 * 7, 42);   // passes: 42 == 42
    ASSERT_EQ(2 + 2, 5);    // FAILS:  4 != 5  →  OOB panic  →  exit code 1
}
