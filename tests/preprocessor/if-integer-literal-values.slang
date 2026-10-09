// Integer literals in `#if` expressions are decoded like literals in code: base prefixes,
// octal, digit separators, and values in the signed 64-bit range that do not fit in 32 bits (#13545).

//TEST:INTERPRET(filecheck=CHECK):
void main()
{
#if 1_000 == 1000
    printf("1_000 == 1000: ok\n");
#else
    printf("1_000 == 1000: FAIL\n");
#endif
    // CHECK: 1_000 == 1000: ok

#if 1_000 != 1
    printf("1_000 != 1: ok\n");
#else
    printf("1_000 != 1: FAIL\n");
#endif
    // CHECK-NEXT: 1_000 != 1: ok

#if 0b101 == 5
    printf("0b101 == 5: ok\n");
#else
    printf("0b101 == 5: FAIL\n");
#endif
    // CHECK-NEXT: 0b101 == 5: ok

#if 0B11 == 3
    printf("0B11 == 3: ok\n");
#else
    printf("0B11 == 3: FAIL\n");
#endif
    // CHECK-NEXT: 0B11 == 3: ok

#if 0X10 == 16
    printf("0X10 == 16: ok\n");
#else
    printf("0X10 == 16: FAIL\n");
#endif
    // CHECK-NEXT: 0X10 == 16: ok

#if 010 == 8
    printf("010 == 8: ok\n");
#else
    printf("010 == 8: FAIL\n");
#endif
    // CHECK-NEXT: 010 == 8: ok

#if 4294967296 > 0
    printf("4294967296 > 0: ok\n");
#else
    printf("4294967296 > 0: FAIL\n");
#endif
    // CHECK-NEXT: 4294967296 > 0: ok

#if 0xFFFFFFFF > 0
    printf("0xFFFFFFFF > 0: ok\n");
#else
    printf("0xFFFFFFFF > 0: FAIL\n");
#endif
    // CHECK-NEXT: 0xFFFFFFFF > 0: ok

#if 0x100000000 > 0
    printf("0x100000000 > 0: ok\n");
#else
    printf("0x100000000 > 0: FAIL\n");
#endif
    // CHECK-NEXT: 0x100000000 > 0: ok
}
