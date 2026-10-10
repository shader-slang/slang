// Overflowing arithmetic in an `#if` expression wraps instead of trapping or invoking undefined
// behavior. The int32-minimum `/ -1` and `% -1` rows are the inputs that crashed the compiler
// with SIGFPE when `#if` evaluated in 32 bits. The INT64_MIN rows reach the `/ -1` and `% -1`
// special cases, where the quotient wraps (as in the GCC and Clang preprocessors) and the
// remainder is 0. The remaining rows pin wrapping `+`, `-`, `*`, unary `-` and shift counts
// reduced modulo 64.

//TEST:INTERPRET(filecheck=CHECK):
void main()
{
#if (-2147483647 - 1) / -1 == 2147483648
    printf("int32 min / -1: ok\n");
#else
    printf("int32 min / -1: FAIL\n");
#endif
    // CHECK: int32 min / -1: ok

#if (-2147483647 - 1) % -1 == 0
    printf("int32 min %% -1: ok\n");
#else
    printf("int32 min %% -1: FAIL\n");
#endif
    // CHECK-NEXT: int32 min % -1: ok

#if (-9223372036854775807 - 1) / -1 == (-9223372036854775807 - 1)
    printf("int64 min / -1: ok\n");
#else
    printf("int64 min / -1: FAIL\n");
#endif
    // CHECK-NEXT: int64 min / -1: ok

#if (-9223372036854775807 - 1) % -1 == 0
    printf("int64 min %% -1: ok\n");
#else
    printf("int64 min %% -1: FAIL\n");
#endif
    // CHECK-NEXT: int64 min % -1: ok

#if 7 / -1 == -7 && 7 % -1 == 0
    printf("7 / -1: ok\n");
#else
    printf("7 / -1: FAIL\n");
#endif
    // CHECK-NEXT: 7 / -1: ok

#if -(-9223372036854775807 - 1) == (-9223372036854775807 - 1)
    printf("negate int64 min: ok\n");
#else
    printf("negate int64 min: FAIL\n");
#endif
    // CHECK-NEXT: negate int64 min: ok

#if 9223372036854775807 + 1 == (-9223372036854775807 - 1)
    printf("int64 max + 1: ok\n");
#else
    printf("int64 max + 1: FAIL\n");
#endif
    // CHECK-NEXT: int64 max + 1: ok

#if 4294967296 * 4294967296 == 0
    printf("2^32 * 2^32: ok\n");
#else
    printf("2^32 * 2^32: FAIL\n");
#endif
    // CHECK-NEXT: 2^32 * 2^32: ok

#if (1 << 64) == 1 && (1 << -1) == (-9223372036854775807 - 1)
    printf("shift count masked: ok\n");
#else
    printf("shift count masked: FAIL\n");
#endif
    // CHECK-NEXT: shift count masked: ok

#if (8 >> 64) == 8 && (8 >> -1) == 0 && (-8 >> 1) == -4 && ((-9223372036854775807 - 1) >> 63) == -1
    printf("rsh: ok\n");
#else
    printf("rsh: FAIL\n");
#endif
    // CHECK-NEXT: rsh: ok

#if (-9223372036854775807 - 2) == 9223372036854775807
    printf("int64 min - 1: ok\n");
#else
    printf("int64 min - 1: FAIL\n");
#endif
    // CHECK-NEXT: int64 min - 1: ok
}
