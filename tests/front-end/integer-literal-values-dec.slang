// Test that decimal integer literal parsing produces correct values
//
// This test assumes 64-bit pointers.

//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int32_t i32;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
    intptr_t iptr;
    uintptr_t uptr;

    // minimum decimal values for the usual types
    i32 = -2147483648;
    printf("'i32=0x%x'\n", i32);
    //CHECK: 'i32=0x80000000'

    u32 = 0U;
    printf("'u32=0x%x'\n", u32);
    //CHECK: 'u32=0x0'

    i64 = -9223372036854775808;
    printf("'i64=0x%llx'\n", i64);
    //CHECK: 'i64=0x8000000000000000'

    u64 = 0U;
    printf("'u64=0x%llx'\n", u64);
    //CHECK: 'u64=0x0'

    iptr = -9223372036854775808Z;
    printf("'iptr=0x%llx'\n", iptr);
    //CHECK: 'iptr=0x8000000000000000'

    uptr = 0UZ;
    printf("'uptr=0x%llx'\n", uptr);
    //CHECK: 'uptr=0x0'


    // decimal values in the middle for the usual types
    i32 = -12345678;
    printf("'i32=0x%x'\n", i32);
    //CHECK: 'i32=0xff439eb2'

    u32 = 123456789U;
    printf("'u32=0x%x'\n", u32);
    //CHECK: 'u32=0x75bcd15'

    i64 = 123456789123456789;
    printf("'i64=0x%llx'\n", i64);
    //CHECK: 'i64=0x1b69b4bacd05f15'

    u64 = 987654321987654321U;
    printf("'u64=0x%llx'\n", u64);
    //CHECK: 'u64=0xdb4da5f7ef412b1'

    iptr = 1375059283656385060Z;
    printf("'iptr=0x%llx'\n", iptr);
    //CHECK: 'iptr=0x1315312213712224'

    uptr = 9167465669930283656UZ;
    printf("'uptr=0x%llx'\n", uptr);
    //CHECK: 'uptr=0x7f3961749ac87288'


    // maximum decimal values for the usual types
    i32 = 2147483647;
    printf("'i32=0x%x'\n", i32);
    //CHECK: 'i32=0x7fffffff'

    u32 = 4294967295U;
    printf("'u32=0x%x'\n", u32);
    //CHECK: 'u32=0xffffffff'

    i64 = 9223372036854775807;
    printf("'i64=0x%llx'\n", i64);
    //CHECK: 'i64=0x7fffffffffffffff'

    u64 = 18446744073709551615U;
    printf("'u64=0x%llx'\n", u64);
    //CHECK: 'u64=0xffffffffffffffff'

    iptr = 9223372036854775807Z;
    printf("'iptr=0x%llx'\n", iptr);
    //CHECK: 'iptr=0x7fffffffffffffff'

    uptr = 18446744073709551615UZ;
    printf("'uptr=0x%llx'\n", uptr);
    //CHECK: 'uptr=0xffffffffffffffff'
}
