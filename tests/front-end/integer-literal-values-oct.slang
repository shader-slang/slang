// Test that octal integer literal parsing produces correct values
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


    // minimum octal values for the usual types
    i32 = 020000000000;
    printf("'i32=%d'\n", i32);
    //CHECK: 'i32=-2147483648'

    u32 = 00U;
    printf("'u32=%u'\n", u32);
    //CHECK: 'u32=0'

    i64 = 01000000000000000000000;
    printf("'i64=%lld'\n", i64);
    //CHECK: 'i64=-9223372036854775808'

    u64 = 00U;
    printf("'u64=%llu'\n", u64);
    //CHECK: 'u64=0'

    iptr = 01000000000000000000000Z;
    printf("'iptr=%lld'\n", iptr);
    //CHECK: 'iptr=-9223372036854775808'

    uptr = 00UZ;
    printf("'uptr=%llu'\n", uptr);
    //CHECK: 'uptr=0'


    // octal values in the middle for the usual types
    i32 = 037720717262;
    printf("'i32=%d'\n", i32);
    //CHECK: 'i32=-12345678'

    u32 = 0726746425U;
    printf("'u32=%u'\n", u32);
    //CHECK: 'u32=123456789'

    i64 = 06664664565464057425;
    printf("'i64=%lld'\n", i64);
    //CHECK: 'i64=123456789123456789'

    u64 = 066646645757675011261U;
    printf("'u64=%llu'\n", u64);
    //CHECK: 'u64=987654321987654321'

    iptr = 0114251422102334221044Z;
    printf("'iptr=%lld'\n", iptr);
    //CHECK: 'iptr=1375059283656385060'

    uptr = 0774713027223262071210UZ;
    printf("'uptr=%llu'\n", uptr);
    //CHECK: 'uptr=9167465669930283656'


    // maximum octal values for the usual types
    i32 = 017777777777;
    printf("'i32=%d'\n", i32);
    //CHECK: 'i32=2147483647'

    u32 = 037777777777U;
    printf("'u32=%u'\n", u32);
    //CHECK: 'u32=4294967295'

    i64 = 0777777777777777777777;
    printf("'i64=%lld'\n", i64);
    //CHECK: 'i64=9223372036854775807'

    u64 = 01777777777777777777777U;
    printf("'u64=%llu'\n", u64);
    //CHECK: 'u64=18446744073709551615'

    iptr = 0777777777777777777777Z;
    printf("'iptr=%lld'\n", iptr);
    //CHECK: 'iptr=9223372036854775807'

    uptr = 01777777777777777777777UZ;
    printf("'uptr=%llu'\n", uptr);
    //CHECK: 'uptr=18446744073709551615'
}
