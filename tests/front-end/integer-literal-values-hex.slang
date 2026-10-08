// Test that hexadecimal integer literal parsing produces correct values
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


    // minimum hexadecimal values for the usual types
    i32 = 0x80000000;
    printf("'i32=%d'\n", i32);
    //CHECK: 'i32=-2147483648'

    u32 = 0x00000000U;
    printf("'u32=%u'\n", u32);
    //CHECK: 'u32=0'

    i64 = 0x8000000000000000;
    printf("'i64=%lld'\n", i64);
    //CHECK: 'i64=-9223372036854775808'

    u64 = 0x0000000000000000U;
    printf("'u64=%llu'\n", u64);
    //CHECK: 'u64=0'

    iptr = 0x8000000000000000Z;
    printf("'iptr=%lld'\n", iptr);
    //CHECK: 'iptr=-9223372036854775808'

    uptr = 0x0000000000000000UZ;
    printf("'uptr=%llu'\n", uptr);
    //CHECK: 'uptr=0'


    // hexadecimal values in the middle for the usual types
    i32 = 0xff439eb2;
    printf("'i32=%d'\n", i32);
    //CHECK: 'i32=-12345678'

    u32 = 0x75bcd15U;
    printf("'u32=%u'\n", u32);
    //CHECK: 'u32=123456789'

    i64 = 0x1b69b4bacd05f15;
    printf("'i64=%lld'\n", i64);
    //CHECK: 'i64=123456789123456789'

    u64 = 0xdb4da5f7ef412b1U;
    printf("'u64=%llu'\n", u64);
    //CHECK: 'u64=987654321987654321'

    iptr = 0x1315312213712224Z;
    printf("'iptr=%lld'\n", iptr);
    //CHECK: 'iptr=1375059283656385060'

    uptr = 0x7f3961749ac87288UZ;
    printf("'uptr=%llu'\n", uptr);
    //CHECK: 'uptr=9167465669930283656'


    // maximum hexadecimal values for the usual types
    i32 = 0x7fffffff;
    printf("'i32=%d'\n", i32);
    //CHECK: 'i32=2147483647'

    u32 = 0xffffffffU;
    printf("'u32=%u'\n", u32);
    //CHECK: 'u32=4294967295'

    i64 = 0x7fffffffffffffff;
    printf("'i64=%lld'\n", i64);
    //CHECK: 'i64=9223372036854775807'

    u64 = 0xffffffffffffffffU;
    printf("'u64=%llu'\n", u64);
    //CHECK: 'u64=18446744073709551615'

    iptr = 0x7fffffffffffffffZ;
    printf("'iptr=%lld'\n", iptr);
    //CHECK: 'iptr=9223372036854775807'

    uptr = 0xffffffffffffffffUZ;
    printf("'uptr=%llu'\n", uptr);
    //CHECK: 'uptr=18446744073709551615'
}
