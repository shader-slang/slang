// Dividing the most negative value by -1 in an `#if` expression must not trap the compiler.
// The quotient wraps (as in GCC and Clang) and the remainder is 0.

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
}
